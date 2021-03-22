// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.21
// Filename:        condition.cc
// Descripton:      实现waitforsecond


#include "dwater/base/condition.h"

#include <errno.h>

// 时间结束返回true否则返回false

bool dwater::Condition::WaitForSeconds(double seconds) {
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);
    const int64_t knano_seconds_per_second = 1000000000;
    int64_t nanoseconds = static_cast<int64_t>(seconds * knano_seconds_per_second);

    abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanoseconds) / knano_seconds_per_second);
    abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nanoseconds) % knano_seconds_per_second);

    dwater::MutexLock::UnassignGuard ug(mutex_);
    return ETIMEDOUT == pthread_cond_timedwait(&pcond_, mutex_.GetPthreadMutex(), &abstime);
}


