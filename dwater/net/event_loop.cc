// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.23
// Filename:        event_loop.cc
// Descripton:       


#include "dwater/net/event_loop.h"

#include "dwater/base/logging.h"
#include "dwater/base/mutex.h"
#include "dwater/net/channel.h"
#include "dwater/net/poller.h"
#include "dwater/net/socket_ops.h"
#include "dwater/net/timer_queue.h"

#include <algorithm>
#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

using namespace dwater;
using namespace dwater::net;

namespace {
__thread EventLoop* t_loop_in_this_thread = 0;

const int kpoll_time_ms = 10000;

int CreateEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if ( evtfd < 0 ) {
        LOG_SYSERR << "Failed in eventfd";
        abort();
    }
    return evtfd;
}


#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe {
public:
    IgnoreSigPipe() {
        ::signal(SIGPIPE, SIG_IGN);
    }
};
#pragma GCC diagnostic ignored  "-Wold-style-cast"

IgnoreSigPipe init_obj;
} // unname namespace 

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      event_handling_(false),
      calling_pending_functors_(false),
      iteration_(0),
      thread_id_(current_thread::Tid()),
      poller_(Poller::NewDefaultPoller(this)),
      timer_queue_(new TimerQueue(this)),
      wakeup_fd_(CreateEventfd()),
      wakeup_channel_(new Channel(this, wakeup_fd_)),
      curr_active_channel_(NULL) {
    
    LOG_DEBUG << "EventLoop created " << this << " in thread" << thread_id_;
    if ( t_loop_in_this_thread ) {
        LOG_FATAL << "Another EventLoop " << t_loop_in_this_thread
            << "exists in this thread " << thread_id_;
    } else {
        t_loop_in_this_thread = this;
    }

    wakeup_channel_->SetReadCallback(std::bind(&EventLoop::HandleRead, this));
    wakeup_channel_->EnableReading();
}

EventLoop::~EventLoop() {
    LOG_DEBUG << "EventLoop  " << this << " of thread " << thread_id_ 
        << " destructs in thread " << current_thread::Tid();
    wakeup_channel_->DisableAll();
    wakeup_channel_->Remove();
    ::close(wakeup_fd_);
    t_loop_in_this_thread = NULL;
}

void EventLoop::Loop() {
    assert(!looping_);
    AssertInLoopThread();
    looping_ = true;
    quit_ = false;
    LOG_TRACE << "EventLoop " << this << " start looping";

    while ( !quit_ ) {
        active_channels_.clear();
        // 条用poller的poll()函数获得活动的 Channels
        poll_return_time_ = poller_->Poll(kpoll_time_ms, &active_channels_);
        ++iteration_;
        if ( Logger::logLevel() <= Logger::TRACE ) {
            PrintActiveChannels();
        }

        event_handling_ = true;
        // 依次调用这些活动Channel的HandleEvent()
        for ( Channel*  channel : active_channels_ ) {
            curr_active_channel_ = channel;
            curr_active_channel_->HandleEvent(poll_return_time_);
        }
        curr_active_channel_ = NULL;
        event_handling_ = false;
        DoPendingFunctors();
    }

    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

// 别的线程会调用这个函数将quit_设置为false，因此loop_停止循环获取事件、处理事件
// 不是马上发生的而是下一轮判断quit_才会停止循环，因此会有延迟
void EventLoop::Quit() {
    quit_ = true;
    if ( !IsInLoopThread() )  {
        Wakeup();
    }
}

void EventLoop::RunInLoop(Functor cb) {
    if ( IsInLoopThread() ) {
        cb();
    } else {
        QueueInLoop(std::move(cb));
    }
}

void EventLoop::QueueInLoop(Functor cb) {
    {
        MutexLockGuard lock(mutex_);
        pending_functors_.push_back(std::move(cb));
    }
    if ( !IsInLoopThread() || calling_pending_functors_ ) {
        Wakeup();
    }
}

size_t EventLoop::QueueSize() const {
    MutexLockGuard lock(mutex_);
    return pending_functors_.size();
}

TimerId EventLoop::RunAt(Timestamp time, TimerCallback cb) {
    return timer_queue_->AddTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::RunAfter(double delay, TimerCallback cb) {
    Timestamp time(AddTime(Timestamp::Now(), delay));
    return RunAt(time, std::move(cb));
}

TimerId EventLoop::RunEvery(double interval, TimerCallback cb) {
    Timestamp time(AddTime(Timestamp::Now(), interval));
    return timer_queue_->AddTimer(std::move(cb), time, interval);
}

void EventLoop::Cancel(TimerId timer_id) {
    timer_queue_->Cancel(timer_id);
}

// 更新工作直接交给poller
void EventLoop::UpdateChannel(Channel* channel) {
    assert(channel->OwnerLoop() == this);
    AssertInLoopThread();
    poller_->UpdateChannel(channel);
}

void EventLoop::RemoveChannel(Channel* channel) {
    assert(channel->OwnerLoop() == this);
    AssertInLoopThread();
    if ( event_handling_ ) {
        assert(curr_active_channel_ == channel || 
            std::find(active_channels_.begin(), active_channels_.end(), channel) == active_channels_.end());
    }
    poller_->RemoveChannel(channel);
}

bool EventLoop::HasChannel(Channel* channel) {
    assert(channel->OwnerLoop() == this);
    AssertInLoopThread();
    return poller_->HasChannel(channel);
}

void EventLoop::AbortNotInLoopThread() {
    LOG_FATAL << "EventLoop::AbortNotInLoopThread -  EventLoop " << this
              << " was created in thread_id_ = " << thread_id_ 
              << ", current thread id = " << current_thread::Tid();
}

// 线程通知机制，其他线程通知IO线程有事件发生
void EventLoop::Wakeup() {
    uint64_t one = 1;
    ssize_t n = socket::Write(wakeup_fd_, &one, sizeof(one));
    if ( n != sizeof(one) ) {
        LOG_ERROR << "EventLoop::Wakeup() writes " << n << " bytes instead 8 bytes";
    }
}

void EventLoop::HandleRead() {
    uint64_t one = 1;
    ssize_t n = socket::Read(wakeup_fd_, &one, sizeof(one));
    if ( n != sizeof(one) ) {
        LOG_ERROR << "EventLoop::HandleRead() reads " << n << " bytes instead of 8 bytes";
    }
}

void EventLoop::DoPendingFunctors() {
    std::vector<Functor> functors;
    calling_pending_functors_ = true;
    {
        MutexLockGuard lock(mutex_);
        functors.swap(pending_functors_);
    }

    for ( const Functor& functor : functors ) {
        functor();
    }
    calling_pending_functors_ = false;
}
void EventLoop::PrintActiveChannels() const {
    for ( const Channel* channel : active_channels_ ) {
        LOG_TRACE << "{" << channel->ReventsToString() << "}";
    }
}
