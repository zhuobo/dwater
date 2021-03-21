// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        condition.h
// Descripton:      封装了条件变量的操作，外部会套一层封装为CountDownLatch
// （倒计时计数器类），设置计数，当倒计时没有结束时，CountDownLatch内部的
// Condition类一直处于Wait()的状态；倒计时结束就可以唤醒线程。两个主要用法：
// 1. 所有子线程等待主线程发起执行命令；
// 2. 主线程等待子线程初始化完毕才开始工作;


#ifndef DWATER_BASE_SRC_CONDITION_H
#define DWATER_BASE_SRC_CONDITION_H

#include "mutex.h"
#include <pthread.h>

namespace dwater {

class Condition : dwater::noncopyable {
private:
    MutexLock&      mutex_;
    pthread_cond_t  pcond_;

public:
    explicit Condition(MutexLock& mutex) : mutex_(mutex) {
        pthread_cond_init(&pcond_, NULL);
    }

    ~Condition() {
        pthread_cond_destroy(&pcond_);
    }

    void Wait() {
        MutexLock::UnassignGuard ug(mutex_);
        pthread_cond_wait(&pcond_, mutex_.GetPthreadMutex());
    }

    // 时间结束就返回true，否则返回false
    bool WaitForSeconds(double seconds);

    void Notify() {
        pthread_cond_signal(&pcond_);
    }

    void NotifyAll() {
        pthread_cond_broadcast(&pcond_);
    }

}; // class Condition

} // namespace dwater


#endif // DWATER_BASE_SRC_CONDITION_H 
