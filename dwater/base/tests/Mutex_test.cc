#include "../mutex.h"

#include <iostream>
#include <thread>

using std::cout;
using std::endl;

void work1(int& sum, dwater::MutexLockGuard& lock_guard) {
    for ( int i = 1; i < 50000000; ++i ) {
        sum += i;
    }
}

void work2(int& sum, dwater::MutexLockGuard& lock_guard) {
    for ( int i = 50000000; i <= 100000000; ++i ) {
        sum += i;
    }
}

int func() {
    int sum = 0;
    for ( int i = 1; i <= 10000; ++i ) {
        sum += i;
    }
    return sum;
}

int main() {
    int res = 0;

    dwater::MutexLock mutex;
    dwater::MutexLockGuard lock_guard(mutex);
    std::thread t1(work1, std::ref(res), std::ref(lock_guard));
    std::thread t2(work2, std::ref(res), std::ref(lock_guard));
    t1.join();
    t2.join();

    cout << "sum1 : " << res << endl;
    cout << "sum2 : " << func() << endl;
    return 0;
}
