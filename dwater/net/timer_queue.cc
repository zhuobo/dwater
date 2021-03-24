// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        timer_queue.cc
// Descripton:       


#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include "dwater/net/timer_queue.h"
#include "dwater/base/logging.h"
#include "dwater/net/event_loop.h"
#include "dwater/net/timer.h"
#include "dwater/net/timerid.h"

#include <sys/timerfd.h>
#include <unistd.h>

namespace dwater {

namespace net {

namespace detail {

int CreateTimerfd() {
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if ( timerfd < 0 ) {
        LOG_SYSFATAL << "Failed in timerfd_create";
    }
    return timerfd;
}

struct timespec HowMuchTimeFromNow(Timestamp when) {
    int64_t microseconds = when.MicroSecondsSinceEpoch() - Timestamp::Now().MicroSecondsSinceEpoch();
    if ( microseconds < 100 ) {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kmicro_seconds_per_second);
    ts.tv_nsec = static_cast<long>((microseconds % Timestamp::kmicro_seconds_per_second) * 1000);
    return ts;
}

/// 
/// 从定时器读
/// 
void ReadTimerfd(int timerfd, Timestamp now) {
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
    LOG_TRACE << "TimerQueue::HandleRead() " << howmany << " at " << now.ToString();
    if ( n != sizeof(howmany) ) {
        LOG_ERROR << "TimerQueue::HandleRead() reads " << n << " bytes instead of 8";
    }
}

/// 
/// 重新设定定时器的超时时间
/// 
void ResetTimerfd(int timerfd, Timestamp expiration) {
    struct itimerspec new_value;
    struct itimerspec old_value;
    MemZero(&new_value, sizeof(new_value));
    MemZero(&old_value, sizeof(old_value));
    new_value.it_value = HowMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &new_value, &old_value);
    if ( ret  ) {
        LOG_SYSFATAL << "timerfd_settime()";
    }
}

} // namespace detail
} // namespace net
} // namespace dwater

using namespace dwater;
using namespace dwater::net;
using namespace dwater::net::detail;

///
/// 初始化属于的EventLoop、创建一个定时器，定时器时间timerfd_channel_,回调函数
/// 
TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop),
      timerfd_(CreateTimerfd()),
      timerfd_channel_(loop, timerfd_),
      timers_(),
      calling_expired_timers_(false) {
    timerfd_channel_.SetReadCallback(std::bind(&TimerQueue::HandleRead, this));
    timerfd_channel_.EnableReading();
}

/// 
/// 移除队列所有的定时器
/// 
TimerQueue::~TimerQueue() {
    timerfd_channel_.DisableAll();
    timerfd_channel_.Remove();
    ::close(timerfd_);

    for ( const Entry& timer : timers_ ) {
        delete timer.second;
    }
}

TimerId TimerQueue::AddTimer(TimerCallback cb, Timestamp when, double interavl) {
    Timer* timer = new Timer(std::move(cb), when, interavl);
    loop_->RunInLoop(std::bind(&TimerQueue::AddTimerInLoop, this, timer));
    return TimerId(timer, timer->Sequence());
}

void TimerQueue::Cancel(TimerId timer_id) {
    loop_->RunInLoop(std::bind(&TimerQueue::CancelInLoop, this, timer_id));
}

void TimerQueue::AddTimerInLoop(Timer* timer) {
    loop_->AssertInLoopThread();
    bool earliest_changed = Insert(timer);
    if ( earliest_changed ) {
        ResetTimerfd(timerfd_, timer->Expiration());
    }
}

void TimerQueue::CancelInLoop(TimerId timer_id) {
    loop_->AssertInLoopThread();
    assert(timers_.size() == active_timers_.size());
    ActiveTimer timer(timer_id.timer_, timer_id.sequence_);
    ActiveTimerSet::iterator it = active_timers_.find(timer);
    if ( it != active_timers_.end() ) {
        size_t n = timers_.erase(Entry(it->first->Expiration(), it->first));
        assert(n == 1);
        (void)n;
        delete it->first;
        active_timers_.erase(it);
    } else if ( calling_expired_timers_ ) {
        canceling_timers_.insert(timer);
    }
    assert(timers_.size() == active_timers_.size());
}

void TimerQueue::HandleRead() {
    loop_->AssertInLoopThread();
    Timestamp now(Timestamp::Now());
    ReadTimerfd(timerfd_, now);
    std::vector<Entry> expired = GetExpired(now);
    calling_expired_timers_ = true;
    canceling_timers_.clear();

    for ( const Entry& it : expired ) {
        it.second->Run(); // run the callback of current timer
    }
    calling_expired_timers_ = false;
    Reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::GetExpired(Timestamp now) {
    assert(timers_.size() == active_timers_.size());
    std::vector<Entry> expired;
    Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    TimerList::iterator end = timers_.lower_bound(sentry);
    assert(end == timers_.end() || now < end->first);
    std::copy(timers_.begin(), end, back_inserter(expired));
    timers_.erase(timers_.begin(), end);

    for (  const Entry& it : expired ) {
        ActiveTimer timer(it.second, it.second->Sequence());
        size_t n = active_timers_.erase(timer);
        assert(n == 1); (void)n;
    }
    assert(timers_.size() == active_timers_.size());
    return expired;
}

void TimerQueue::Reset(const std::vector<Entry>& expired, Timestamp now) {
    Timestamp next_expired;
    for ( const Entry& it : expired ) {
        ActiveTimer timer(it.second, it.second->Sequence());
        if ( it.second->Repeat() && canceling_timers_.find(timer) == canceling_timers_.end()) {
            it.second->Restart(now); // 重启再次插入
            Insert(it.second);
        } else {
            delete it.second; // 到时且不是repeat的定时器要删除
        }
    }
    if ( !timers_.empty() ) {
        next_expired = timers_.begin()->second->Expiration();
    }
    if ( next_expired.Valid() ) {
        ResetTimerfd(timerfd_, next_expired);
    }
}

bool TimerQueue::Insert(Timer* timer) {
    loop_->AssertInLoopThread();
    assert(timers_.size() == active_timers_.size());
    bool earliest_changed = false;
    Timestamp when = timer->Expiration();
    TimerList::iterator it = timers_.begin();
    if ( it == timers_.end() || when < it->first ) {
        earliest_changed = true;
    }
    {
        std::pair<TimerList::iterator, bool> result = timers_.insert(Entry(when, timer));
        assert(result.second); (void)result;
    }
    {
        std::pair<ActiveTimerSet::iterator, bool> result = active_timers_.insert(ActiveTimer(timer, timer->Sequence()));
        assert(result.second); (void)result;
    }
    assert(timers_.size() == active_timers_.size());
    return earliest_changed;
}
