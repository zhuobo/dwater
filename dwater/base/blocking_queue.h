// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.20
// Filename:        blocking_queue.h
// Descripton:      无界阻塞队列，生产者消费者队列

#ifndef DWATER_SRC_BASE_BLOCKING_QUEUE_H
#define DWATER_SRC_BASE_BLOCKING_QUEUE_H

#include "dwater/base/condition.h"
#include "dwater/base/mutex.h"

#include <deque>
#include <assert.h>


namespace dwater {

template<typename T>
class BlockingQueue : noncopyable {
public:
    BlockingQueue() : mutex_(), not_empty_(mutex_), queue_() {}

    void Put(const T& x) {
        MutexLockGuard lock(mutex_);
        queue_.push_back(x);
        not_empty_.Notify();
    }

    // 右值引用，移动语义，减少拷贝的开销
    void Put(T&& x) {
        MutexLockGuard lock(mutex_);
        queue_.push_back(std::move(x));
        not_empty_.Notify();
    }

    T Take() {
        MutexLockGuard lock(mutex_);
        while ( queue_.empty() ) {
            not_empty_.Wait();
        }
        assert(!queue_.empty());
        T front(std::move(queue_.front()));
        queue_.pop_front();
        return front;
    }

    size_t Size() const {
        MutexLockGuard lock(mutex_);
        return queue_.size();
    }

private:
    mutable MutexLock mutex_;
    Condition not_empty_ GUARDED_BY(mutex_);
    std::deque<T> queue_ GUARDED_BY(mutex_);
};

} // namespace dwater

#endif // DWATER_SRC_BASE_BLOCKING_QUEUE_H
