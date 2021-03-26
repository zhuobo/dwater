// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.24
// Filename:        event_loop_thread.h
// Descripton:       

#ifndef DWATER_NET_EVENT_LOOP_THREAD_H
#define DWATER_NET_EVENT_LOOP_THREAD_H

#include "dwater/base/condition.h"
#include "dwater/base/mutex.h"
#include "dwater/base/thread.h"

namespace dwater {

namespace net {

class EventLoop;

/// 
/// 封装了IO线程，返回本线程的EventLoop对象的指针
/// 
class EventLoopThread : noncopyable {
public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;

    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                    const string& name = string());
    ~EventLoopThread();
    
    ///
    /// 开始循环，并返回EventLoop对象的指针
    ///
    EventLoop* StartLoop();

private:
    void ThreadFunc();

    EventLoop*  loop_ GUARDED_BY(mutex_);
    bool        exiting_;
    Thread      thread_;
    MutexLock   mutex_;
    Condition   cond_;
    ThreadInitCallback callback_;
};

} // namespace dwater

} // namespace dwater




#endif // DWATER_NET_EVENT_LOOP_THREAD_H



