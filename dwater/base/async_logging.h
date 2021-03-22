// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.21
// Filename:        async_logging.h
// Descripton:      异步日志

#ifndef DWATER_SRC_BASE_ASYNC_LOGGING_H
#define DWATER_SRC_BASE_ASYNC_LOGGING_H

#include "dwater/base/blocking_queue.h"
#include "dwater/base/bounded_blocking_queue.h"
#include "dwater/base/count_down_latch.h"
#include "dwater/base/mutex.h"
#include "dwater/base/thread.h"
#include "dwater/base/log_stream.h"

#include <atomic>
#include <vector>

namespace dwater {

class AsyncLogging : noncopyable {
public:
    AsyncLogging(const string& basename, off_t rool_size, int flush_interval = 3);

    ~AsyncLogging() {
        if ( running_ ) {
            Stop(); 
        }
    }

    // 所有的LOG_xx最终都会调用这个函数，尽管往两个buffer些数据
    void Append(const char* logline, int len);

    void Start() {
        running_ = true;
        thread_.Start();
        latch_.Wait();
    }

    void Stop() NO_THREAD_SAFETY_ANALYSIS {
        running_ = false;
        cond_.Notify();
        thread_.Join();
    }

private:
    void ThreadFunc();

    typedef dwater::detail::FixedBuffer<dwater::detail::k_large_buffer> Buffer;
    typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
    typedef BufferVector::value_type BufferPtr;

    const int flush_interval_;
    std::atomic<bool> running_;
    const string basename_;
    const off_t roll_size_;
    dwater::Thread thread_;
    dwater::CountDownLatch latch_;
    dwater::MutexLock mutex_;
    dwater::Condition cond_ GUARDED_BY(mutex_);
    BufferPtr curr_buffer_ GUARDED_BY(mutex_);
    BufferPtr next_buffer_ GUARDED_BY(mutex_);
    BufferVector buffers_ GUARDED_BY(mutex_);
}; // class AsyncLogging

} // namespace dwater 

#endif // DWATER_SRC_BASE_ASYNC_LOGGING_H


