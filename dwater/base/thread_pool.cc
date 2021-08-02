// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.04.09
// Filename:        thread_pool.cc
// Descripton:       

#include "dwater/base/thread_pool.h"

#include "dwater/base/exception.h"

#include <assert.h>
#include <stdio.h>

using namespace dwater;

ThreadPool::ThreadPool(const string& name)
    : mutex_(),
      not_empty_(mutex_),
      not_full_(mutex_),
      name_(name),
      max_queue_size_(0),
      running_(false) {}

ThreadPool::~ThreadPool() {
    if ( running_ ) {
        Stop();
    }
}

void ThreadPool::Start(int num_threads) {
    assert(threads_.empty());
    running_ = true;
    threads_.reserve(num_threads);
    for ( int i = 0; i < num_threads; ++i ) {
        char id[32];
        snprintf(id, sizeof(id), "%d", i + 1);
        threads_.emplace_back(new dwater::Thread(std::bind(&ThreadPool::RunInThread, this), name_ + id));
        threads_[i]->Start();
    }
    if ( num_threads == 0 && thread_init_callback_ ) {
        thread_init_callback_();
    }
}

// 等待所有线程都执行完毕
void ThreadPool::Stop() {
    { 
        MutexLockGuard lock(mutex_);
        running_  = false;
        not_empty_.NotifyAll();
        not_full_.NotifyAll();
    }

    for ( auto& thread : threads_ ) {
        thread->Join();
    }
}

size_t ThreadPool::QueueSize() const {
    MutexLockGuard lock(mutex_);
    return queue_.size();
}

// 并非真的执行，而是加入任务队列
void ThreadPool::Run(Task task) {
    if ( threads_.empty() ) {
        task(); // 如果没有子线程，那么就直接在主线程执行
    } else {
        MutexLockGuard lock(mutex_);
        while ( IsFull() && running_ ) {
            not_full_.Wait();
        }
        if ( !running_ ) {
            return;
        }
        assert(!IsFull());
        queue_.push_back(std::move(task));
        not_empty_.Notify(); // 通知某个线程任务队列非空了
    }
}

// 从任务队列取出一个任务并返回
ThreadPool::Task ThreadPool::Take() {
    MutexLockGuard lock(mutex_);
    while ( queue_.empty() && running_ ) {
        not_empty_.Wait();
    }
    Task task;
    if ( !queue_.empty() ) {
        task = queue_.front();
        queue_.pop_front();
        if ( max_queue_size_ > 0 ) {
            not_full_.Notify();
        }
    }
    return task;
}

bool ThreadPool::IsFull() const {
    mutex_.AssertLocked();
    return max_queue_size_ > 0 && queue_.size() >= max_queue_size_;
}

// 从任务队列取出一个任务来执行
void  ThreadPool::RunInThread() {
    try {
        if ( thread_init_callback_ ) {
            thread_init_callback_();
        }
        while ( running_ ) {
            Task task(Take());
            if ( task ) {
                task();
            }
        }
    } catch (const Exception& ex) {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what()) ;
        fprintf(stderr, "stack trace: %s\n", ex.StackTrace());
        abort();
    } catch (const std::exception& ex) {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        abort();
    } catch (...) {
        fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
        throw; // rethrow
    }
}
