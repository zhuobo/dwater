// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        count_down_latch.h
// Descripton:      封装Condition类，作为一个倒计时器类，功能主要有两个：
// 1. 主线程发起多个子线程，等待这些子线程都完成一定的任务后(一般都是子线程的初
// 始化)，主线程才开始执行
// 2. 主线程发起多个子线程，等主线程完成一定的任务后，通知所有子线程开始执行，
// 也就是子线程等待主线程发起“起跑命令”


#ifndef WATER_BASE_SRC_COUNT_DOWN_LATCH_H
#define WATER_BASE_SRC_COUNT_DOWN_LATCH_H

#include "dwater/base/condition.h"
#include "dwater/base/mutex.h"

namespace dwater {

class CountDownLatch : noncopyable
{
public:

    explicit CountDownLatch(int count);

    void Wait();

    void CountDown();

    int GetCount() const;

private:
    mutable MutexLock mutex_;
    Condition condition_ GUARDED_BY(mutex_);
    int count_ GUARDED_BY(mutex_);
};


} // namespace dwater

#endif // WATER_BASE_SRC_COUNT_DOWN_LATCH_H
