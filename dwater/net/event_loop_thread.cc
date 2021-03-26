// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.24
// Filename:        event_loop_thread.cc
// Descripton:       

#include "dwater/net/event_loop_thread.h"
#include "dwater/net/event_loop.h"

using namespace dwater;
using namespace dwater::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                 const string& name)
    : loop_(NULL),
      exiting_(false),
      thread_(std::bind(&EventLoopThread::ThreadFunc, this), name),
      mutex_(),
      cond_(mutex_),
      callback_(cb) {}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if ( loop_ != NULL ) {
        loop_->Quit();
        thread_.Join();
    }
}

EventLoop* EventLoopThread::StartLoop() {
    assert(!thread_.Started());
    thread_.Start();

    EventLoop* loop = NULL;
    {
        MutexLockGuard lock(mutex_);
        while ( loop_ == NULL ) {
            cond_.Wait();
        }
        loop = loop_;
    }
    return loop_;
}

void EventLoopThread::ThreadFunc() {
    EventLoop loop;
    if ( callback_ ) {
        callback_(&loop);
    }
    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        cond_.Notify();
    }
    loop.Loop();
    MutexLockGuard lock(mutex_);
    loop_ = NULL;
}
