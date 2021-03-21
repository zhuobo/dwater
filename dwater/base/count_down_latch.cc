// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        count_down_latch.cc
// Descripton:      CountDownLatch类的实现

#include "count_down_latch.h"

using namespace dwater;

CountDownLatch::CountDownLatch(int count)
    : mutex_(),
      condition_(mutex_),
      count_(count) {
}

void CountDownLatch::Wait() {
    MutexLockGuard lock(mutex_);
    while ( count_ > 0 ) {
        condition_.Wait();
    }
}

void CountDownLatch::CountDown() {
    MutexLockGuard lock(mutex_);
    --count_;
    if ( count_ == 0 ) {
        condition_.NotifyAll();
    }
}

int CountDownLatch::GetCount() const {
    MutexLockGuard lock(mutex_);
    return count_;
}



