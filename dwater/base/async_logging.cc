// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.21
// Filename:        async_logging.cc
// Descripton:      异步日志的实现

#include "async_logging.h"
#include "log_file.h"
#include "condition.h"
#include "timestamp.h"

#include <stdio.h>

using namespace dwater;

// 异步日志中有连个FixedBuffer,以及一个Buffervector,重点在于Append数据

// typedef dwater::detail::FixedBuffer<dwater::detail::k_large_buffer> Buffer;
// typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
// typedef BufferVector::value_type BufferPtr;

// const int flush_interval_;
// std::atomic<bool> running_;
// const string basename_;
// const off_t roll_size_;
// dwater::Thread thread_; // 后台线程，这个线程消化buffers_中的数据，写到LogFile中，再由LogFile进行IO
// dwater::CountDownLatch latch_;
// dwater::MutexLock mutex_;
// dwater::Condition cond_ GUARDED_BY(mutex_);
// BufferPtr curr_buffer_ GUARDED_BY(mutex_);
// BufferPtr next_buffer_ GUARDED_BY(mutex_);
// BufferVector buffers_ GUARDED_BY(mutex_);

AsyncLogging::AsyncLogging(const string& basename,
                          off_t roll_size,
                          int flush_interval)
    : flush_interval_(flush_interval),
      running_(false),
      basename_(basename),
      roll_size_(roll_size),
      thread_(std::bind(&AsyncLogging::ThreadFunc, this), "Logging"),
      latch_(1),
      mutex_(),
      cond_(mutex_),
      curr_buffer_(new Buffer),
      next_buffer_(new Buffer),
      buffers_() {
    curr_buffer_->Bzero();
    next_buffer_->Bzero();
    buffers_.reserve(16);
}

// 使用mutex_保护两个buffer的读写，当curr_buf没有满可以写数据就直接写，否则
// 就将curr_buf插入到bufvec中，把next_buf的数据移动到curr_buf, 如果next_buf也没有
// 数据，那么直接给curr_buf申请新的空间.在这里curr_buf是作为临时的中转的
// 数据写完之后通过条件变量通知其他线程
void AsyncLogging::Append(const char* logline, int len) {
    MutexLockGuard lock(mutex_);
    if ( curr_buffer_->Avail() ) {
        curr_buffer_->Append(logline, len);
    } else {
        buffers_.push_back(std::move(curr_buffer_));
        if ( next_buffer_ ) {
            curr_buffer_ = std::move(next_buffer_);
        } else {
            curr_buffer_.reset(new Buffer);
        }
        curr_buffer_->Append(logline, len);
        cond_.Notify(); // 至少一个buffer满了就通知后天线程要写数据到磁盘了
    }
}

// 后台线程，消化buffervector的数据，将数据写到磁盘
// 给一个线程专门执行这个函数
void AsyncLogging::ThreadFunc() {
    assert(running_ == true);
    latch_.CountDown();
    LogFile output(basename_, roll_size_, false); // 由logFile类直接进行IO
    BufferPtr new_buffer1(new Buffer); // 后台线程的两个buffer
    BufferPtr new_buffer2(new Buffer);
    new_buffer1->Bzero();
    new_buffer2->Bzero();
    BufferVector buffers_to_write;  // 将前台产生buffervector中的数据swap过来
    buffers_to_write.reserve(16);

    while ( running_ ) {
        assert(new_buffer1 && new_buffer1->Length() == 0);
        assert(new_buffer2 && new_buffer2->Length() == 0);
        assert(buffers_to_write.empty());

        {
            MutexLockGuard lock(mutex_);
            // 等待醒过来，无论因为时间到了还是条件变量好了，都会醒过来
            if ( buffers_.empty() ) {
                cond_.WaitForSeconds(flush_interval_);
            }
            // curr_buff_数据放入到buffers_中
            buffers_.push_back(std::move(curr_buffer_));
            // 还一个空的curr_buffer给前台
            curr_buffer_ = std::move(new_buffer1);
            // 把此时所有的数据移动到后台线程自己的buffer中
            buffers_to_write.swap(buffers_);
            // 如果第二个也是空的话，那么第二个也归还
            if ( !next_buffer_ ) {
                next_buffer_ = std::move(new_buffer2);
            }
        }

        // 如果要写的数据太多了，就丢弃部分数据，只丢下前连个buffer写到文件
        assert(!buffers_to_write.empty());
        if ( buffers_to_write.size() > 25 ) {
            char buf[256];
            snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
                    Timestamp::Now().ToFormattedString().c_str(),
                    buffers_to_write.size() - 2);
            fputs(buf, stderr);
            output.Append(buf, static_cast<int>(strlen(buf)));
            buffers_to_write.erase(buffers_to_write.begin() + 2, buffers_to_write.end());
        }

        // 写到LogFile中，直接写到文件
        for ( const auto& buffer : buffers_to_write ) {
            output.Append(buffer->Data(), buffer->Length());
        }

        if ( buffers_to_write.size() > 2 ) {
            buffers_to_write.resize(2);
        }

        // 写完了清空数据
        if ( !new_buffer1 ) {
            assert(!buffers_to_write.empty());
            new_buffer1 = std::move(buffers_to_write.back());
            buffers_to_write.pop_back();
            new_buffer1.reset();
        }

        if ( !new_buffer2 ) {
            assert(!buffers_to_write.empty());
            new_buffer2 = std::move(buffers_to_write.back());
            buffers_to_write.pop_back();
            new_buffer2.reset();
        }

        buffers_to_write.clear();
        output.Flush();
    }
    // 最后LogFile刷写
    output.Flush();
}
