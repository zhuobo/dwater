// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.20
// Filename:        bounded_blocking_queue.h
// Descripton:      有界的队列,使用boost的循环队列


#ifndef DWATER_SRC_BASE_BOUNDED_BLOCKING_QUEUE_H
#define DWATER_SRC_BASE_BOUNDED_BLOCKING_QUEUE_H

#include "mutex.h"
#include "condition.h"

#include <boost/circular_buffer.hpp>
#include <assert.h>

namespace dwater {

template<typename T>
class BoundedBlockingQueue : noncopyable {
public:
    explicit BoundedBlockingQueue(int maxSize)
        : mutex_(),
        not_empty_(mutex_),
        not_full_(mutex_),
        queue_(maxSize){}

    void Put(const T& x) {
        MutexLockGuard lock(mutex_);
        while (queue_.full()) {
            not_full_.Wait();
        }
        assert(!queue_.full());
        queue_.push_back(x);
        not_empty_.Notify();
    }

    void Put(T&& x) {
        MutexLockGuard lock(mutex_);
        while (queue_.full()) {
            not_full_.Wait();
        }
        assert(!queue_.full());
        queue_.push_back(std::move(x));
        not_empty_.Notify();
        }

    T Take() {
    MutexLockGuard lock(mutex_);
        while (queue_.empty()) {
            not_empty_.Wait();
        }
        assert(!queue_.empty());
        T front(std::move(queue_.front()));
        queue_.pop_front();
        not_full_.Notify();
        return front;
    }

    bool Empty() const {
        MutexLockGuard lock(mutex_);
        return queue_.empty();
    }

    bool Full() const {
        MutexLockGuard lock(mutex_);
        return queue_.full();
    }

    size_t Size() const {
        MutexLockGuard lock(mutex_);
        return queue_.size();
    }

    size_t Capacity() const {
        MutexLockGuard lock(mutex_);
        return queue_.capacity();
    }

    private:
    mutable MutexLock          mutex_;
    Condition                  not_empty_ GUARDED_BY(mutex_);
    Condition                  not_full_ GUARDED_BY(mutex_);
    boost::circular_buffer<T>  queue_ GUARDED_BY(mutex_);

}; // class  BoundedBlockingQueue

} // namespace dwater

#endif // DWATER_SRC_BASE_BOUNDED_BLOCKING_QUEUE_H
