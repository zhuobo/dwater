// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        thread.h
// Descripton:      Thread类是对线程的封装，提供线程的各种操作，而且提供线程安全
// 的观察者模式，同时使用智能指针shared_ptr和weak_ptr进行线程生命周期的管理

#ifndef DWATER_SRC_BASE_THREAD_H
#define DWATER_SRC_BASE_THREAD_H

#include "dwater/base/atomic.h"
#include "dwater/base/count_down_latch.h"
#include "dwater/base/types.h"

#include <functional>
#include <memory>
#include <pthread.h>

namespace dwater {

class Thread : noncopyable {
public:
    // 线程的回调函数
    typedef std::function<void ()> ThreadFunc;

private:
    bool            started_;
    bool            joined_;
    pthread_t       pthread_id_; // 线程id这个id在不同进程下可能会一样
    pid_t           tid_; // 因此需要一个真的线程tid
    ThreadFunc      func_;
    string          name_; // 线程名字
    CountDownLatch  latch_;

    static AtomicInt32 num_created_; // 32位整数原子操作，线程数量

private:
    void SetDefaultName();

public:
    explicit Thread(ThreadFunc, const string& name = string());

    ~Thread();

    void Start(); // 最终调用thread_create

    int Join(); // 调用thread_join

    bool Started() const { return started_; }

    pid_t tid() const { return tid_; }

    const string& name() const { return name_; }

    static int NumCreated() { return num_created_.Get(); }

}; // class Thread

} // namespace dwater

#endif // DWATER_SRC_BASE_THREAD_H 
