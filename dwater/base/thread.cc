// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.19
// Filename:        thread.cc
// Descripton:      

#include "thread.h"

#include "current_thread.h"
#include "exception.h"
#include "logging.h"

#include <type_traits>
#include <errno.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

namespace dwater {

namespace detail {

// 获取当前线程的进程pid,glibc没有实现getpid(),通过Linux的系统调用获得
// 这是线程真实id可用在不同进程间的线程通信
pid_t Gettid() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

void AfterFork() {
    dwater::current_thread::t_cached_tid = 0;
    dwater::current_thread::t_thread_name = "main";
    current_thread::Tid();
}

class ThreadnameInitializer {
public:
    ThreadnameInitializer() {
        dwater::current_thread::t_thread_name = "main";
        current_thread::Tid();
        pthread_atfork(NULL, NULL, &AfterFork);
    }

}; // class ThreadnameInitializer

ThreadnameInitializer init;

// 创建线程的一个跳板，这里封装了线程的id，名字，线程函数，等信息
struct ThreadData {
    typedef dwater::Thread::ThreadFunc ThreadFunc;
    ThreadFunc func_; // 创建线程真正执行的函数
    string name_; 
    pid_t* tid_;
    CountDownLatch* latch_;

    ThreadData(ThreadFunc func, const string& name, pid_t* tid, CountDownLatch* latch)
        :   func_(std::move(func)),
            name_(name),
            tid_(tid),
            latch_(latch) {  }

    // 调用func_,执行线程函数
    void RunInThread() {
        *tid_ = dwater::current_thread::Tid();
        tid_ = NULL;
        latch_->CountDown();
        latch_ = NULL;

        dwater::current_thread::t_thread_name = name_.empty() ? "dwaterThread" : name_.c_str();
        ::prctl(PR_SET_NAME, dwater::current_thread::t_thread_name);

        try {
            func_();
            dwater::current_thread::t_thread_name = "finished";
        } catch (const Exception& ex) {
            dwater::current_thread::t_thread_name = "crashed";
            fprintf(stderr, "exception cauught in thread %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            fprintf(stderr, "stack trace: %s\n", ex.StackTrace());
            abort();
        } catch ( const std::exception& ex ) {
            dwater::current_thread::t_thread_name = "crashed";
            fprintf(stderr, "exception cauught in thread %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            abort();
        } catch(...) {
            dwater::current_thread::t_thread_name = "crashed";
            fprintf(stderr, "unknown exception caught in thread %s\n", name_.c_str());
            throw;
        }
    }
};

// 创建线程的回调函数，由Start()调用,参数转化为threaddata指针，然后执行runinthread
void* StartThread(void* object) {
    ThreadData* data = static_cast<ThreadData*>(object);
    data->RunInThread();
    delete data;
    return NULL;
}

} // namesapce detail

// 
void current_thread::CacheTid() {
    if ( t_cached_tid == 0 ) {
        t_cached_tid = detail::Gettid();
        t_tid_string_length = snprintf(t_tid_string, sizeof t_tid_string, "%5d", t_cached_tid);
    }
}

bool current_thread::IsMainThread() {
    return Tid() == ::getpid();
}

void current_thread::SleepUsec(int64_t usec) {
    struct timespec ts = { 0, 0 };
    ts.tv_sec = static_cast<time_t>(usec / Timestamp::kmicro_seconds_per_second);
    ts.tv_nsec = static_cast<long>(usec % Timestamp::kmicro_seconds_per_second * 1000);
    ::nanosleep(&ts, NULL);
}

AtomicInt32 Thread::num_created_;

Thread::Thread(ThreadFunc func, const string& str)
    :   started_(false),
        joined_(false),
        pthread_id_(0),
        tid_(0),
        func_(std::move(func)),
        name_(str),
        latch_(1) {
    SetDefaultName();            
}

Thread::~Thread() {
    if ( started_ && !joined_ ) {
        pthread_detach(pthread_id_);
    }
}

void Thread::SetDefaultName() {
    int num = num_created_.IncrementAndGet();
    if ( name_.empty() ) {
        char buf[32];
        snprintf(buf, sizeof(buf), "Thread%d", num);
        name_ = buf;
    }
}

// 创建一个线程并执行由投资构造函数指定的线程函数
void Thread::Start() {
    assert(!started_);
    started_ = true;
    detail::ThreadData* data = new detail::ThreadData(func_, name_, &tid_, &latch_);
    // 创建一个新线程，并传递一个data给 函数StartThread
    if ( pthread_create(&pthread_id_, NULL, &detail::StartThread, data) ) {
        started_ = false;
        delete data;
        LOG_SYSFATAL << "Failed in pthread_create";
    } else {
        latch_.Wait();
        assert(tid_ > 0);
    }
}

// 调用pthread_join将当前线
int Thread::Join() {
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthread_id_, NULL);
}

} // namspace dwater
