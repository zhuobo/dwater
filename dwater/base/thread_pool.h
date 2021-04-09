// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.04.09
// Filename:        thread_pool.h
// Descripton:       

#ifndef DWATER_BASE_THREAD_POOL_H
#define DWATER_BASE_THREAD_POOL_H

#include "dwater/base/condition.h"
#include "dwater/base/mutex.h"
#include "dwater/base/thread.h"
#include "dwater/base/types.h"

#include <deque>
#include <vector>

namespace dwater {

class ThreadPool : noncopyable {
public:
    typedef std::function<void ()> Task;

    explicit ThreadPool(const string& name = string("ThreadPool"));

    ~ThreadPool();

    void SetMaxQueueSize(int max_size) {
        max_queue_size_ = max_size;
    }

    void SetThreadInitCallback(const Task& cb) {
        thread_init_callback_ = cb;
    }

    void Start(int num_thread);

    void Stop();

    const string& Name() const {
        return name_;
    }

    size_t QueueSize() const;

    void Run(Task task);

private:
    bool IsFull() const REQUIRES(mutex_);

    void RunInThread();
    Task Take();

    mutable MutexLock mutex_;
    Condition not_empty_ GUARDED_BY(mutex_);
    Condition not_full_ GUARDED_BY(mutex_);
    string name_;
    Task thread_init_callback_;
    std::vector<std::unique_ptr<dwater::Thread>> threads_;
    std::deque<Task> queue_ GUARDED_BY(mutex_);
    size_t max_queue_size_;
    bool running_;
};
} // namespace dwater

#endif //  DWATER_BASE_THREAD_POOL_H

