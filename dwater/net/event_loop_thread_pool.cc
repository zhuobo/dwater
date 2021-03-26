// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.26
// Filename:        event_loop_thread_pool.cc
// Descripton:       


#include "dwater/net/event_loop_thread_pool.h"

#include "dwater/net/event_loop.h"
#include "dwater/net/event_loop_thread.h"

#include <stdio.h>

using namespace dwater;
using namespace dwater::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* base_loop, const string& name)
    : base_loop_(base_loop),
      name_(name),
      started_(false),
      num_thread_(0),
      next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {
// 线程池不会被删除
}

void EventLoopThreadPool::Start(const ThreadInitCallback& cb) {
    assert(!started_);
    base_loop_->AssertInLoopThread();
    started_ = true;
    for ( int i = 0; i < num_thread_; ++i ) {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
        EventLoopThread* t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->StartLoop());
    }
    if ( num_thread_ == 0 && cb ) {
        cb(base_loop_);
    }
}

EventLoop* EventLoopThreadPool::GetNextLoop() {
    base_loop_->AssertInLoopThread();
    assert(started_);
    EventLoop* loop = base_loop_;
    if ( !loops_.empty() ) {
        loop = loops_[next_];
        ++next_;
        if ( implicit_cast<size_t>(next_) >= loops_.size() ) {
            next_ = 0;
        }
    }
    return loop;
}

EventLoop* EventLoopThreadPool::GetLoopForHash(size_t hash_code) {
    base_loop_->AssertInLoopThread();
    EventLoop* loop = base_loop_;
    if ( !loops_.empty() ) {
        loop = loops_[hash_code % loops_.size()];
    }
    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::GetAllLoops() {
    base_loop_->AssertInLoopThread();
    assert(started_);
    if ( loops_.empty() ) {
        return std::vector<EventLoop*>(1, base_loop_);
    } else {
        return loops_;
    }
}
