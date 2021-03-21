////////////////////////////////////////////////////////////////////////////////
// MutexLock and MutexLockGuard: RAII skills;
// MutexLock is a member variable of MutexLockGuard, using MutexLockGuard to
// encapsulate the code critical area does not need to be manually unlocked.
// Because MutexLockGuard will be automatically destroyed after leaving the
// active area, execute its destructor, and automatically unlock.
// ///////////////////////////////////////////////////////////////////////////

#ifndef DWATER_SRC_BASE_MUTEX_H
#define DWATER_SRC_BASE_MUTEX_H

#include "noncopable.h"
#include "current_thread.h"
#include <assert.h>
#include <pthread.h>

#if defined(__clang__) && (!defined(SWIG))
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   // no-op
#endif

#define CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(capability(x))

#define SCOPED_CAPABILITY \
  THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

#define GUARDED_BY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define PT_GUARDED_BY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#define ACQUIRED_BEFORE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

#define ACQUIRED_AFTER(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

#define REQUIRES(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

#define REQUIRES_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))

#define ACQUIRE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))

#define ACQUIRE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

#define RELEASE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))

#define RELEASE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

#define TRY_ACQUIRE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))

#define TRY_ACQUIRE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))

#define EXCLUDES(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define ASSERT_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))

#define ASSERT_SHARED_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

#define RETURN_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define NO_THREAD_SAFETY_ANALYSIS \
  THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)

// End of thread safety annotations }

#ifdef CHECK_PTHREAD_RETURN_VALUE

#ifdef NDEBUG

__BEGIN_DECLS
extern void __assert_perror_fail (int errnum,
                                  const char *file,
                                  unsigned int line,
                                  const char *function)
    noexcept __attribute__ ((__noreturn__));
__END_DECLS
#endif

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       if (__builtin_expect(errnum != 0, 0))    \
                         __assert_perror_fail (errnum, __FILE__, __LINE__, __func__);})

#else  // CHECK_PTHREAD_RETURN_VALUE

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       assert(errnum == 0); (void) errnum;})

#endif // CHECK_PTHREAD_RETURN_VALUE



namespace dwater {

class MutexLock : dwater::noncopyable {
private:
    pthread_mutex_t mutex_;
    pid_t           holder_;

private:
    friend class Condition;

private:
    class UnassignGuard : dwater::noncopyable {
    private:
        MutexLock& owner_;
    
    public:
        explicit UnassignGuard(MutexLock& owner) : owner_(owner) {
            owner_.UnassignHolder();
        }

        ~UnassignGuard() {
            owner_.AssignHolder();
        }
    };

    void UnassignHolder() {
        holder_ = 0;
    }

    void AssignHolder() {
        holder_ = dwater::current_thread::Tid();
    }

public:
    MutexLock() : holder_(0) {
        pthread_mutex_init(&mutex_, NULL);
    }

    ~MutexLock() {
        assert(holder_ == 0);
        pthread_mutex_destroy(&mutex_);
    }

    bool IsLockedByThisThread() const {
        return holder_  == dwater::current_thread::Tid();
    }

    void AssertLocked() const {
        assert(IsLockedByThisThread());
    }

    void Lock() {
        pthread_mutex_lock(&mutex_);
        AssignHolder();
    }

    void Unlock() {
        UnassignHolder();
        pthread_mutex_unlock(&mutex_);
    }

    pthread_mutex_t* GetPthreadMutex() {
        return &mutex_;
    }
};  // class MutexLock

// MutexLockGuard被创建的时候就同时创建了一个MutexLock，这个栈上的MutexLockGuard
// 由于离开作用区域被销毁，内部的析构函数自动MutexLock对象自动释放锁，此所谓RAII
class MutexLockGuard : dwater::noncopyable {
private:
    MutexLock &mutex_;

public:
    explicit MutexLockGuard(MutexLock &mutex) : mutex_(mutex) {
        mutex_.Lock();
    }

    ~MutexLockGuard() {
        mutex_.Unlock();
    }
}; // class MutexLockGuard

}  // namespace dwater
#endif // DWATER_SRC_BASE_MUTEX_H
