// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        atomic.h
// Descripton:      用于实现数据的原子操作，统计数据的时候可以不被线程的调度打乱
// 相比于mutex，原子操作的开销更小。这个文件在项目中用于统计字节数以及消息数量
#ifndef DWATER_SRC_BASE_ATOMIC_H
#define DWATER_SRC_BASE_ATOMIC_H

#include <stdint.h>

#include "dwater/base/noncopable.h"

namespace dwater {

namespace detail {

template<typename T>
class AtomicIntegerT : dwater::noncopyable {
private:
    volatile T value_; // shared value_
public:
    T Get() {
        return __sync_val_compare_and_swap(&value_, 0, 0);
    }

    T GetAndAdd(T x) {
        return __sync_fetch_and_add(&value_, x);
    }

    T AddAndGet(T x) {
        return GetAndAdd(x) + x;
    }

    T IncrementAndGet() {
        return AddAndGet(1);
    }

    T DecrementAndGet() {
        return AddAndGet(1);
    }

    void Add(T x) {
        GetAndAdd(x);
    }

    void Increment() {
        IncrementAndGet();
    }
    
    void Decrement() {
        DecrementAndGet();
    }

    T GetAndSet(T new_value) {
        
        return __sync_lock_test_and_set(&value_, new_value);
    }

}; // class template AtomicIntegerT

} // namespace detail
typedef detail::AtomicIntegerT<int32_t> AtomicInt32;
typedef detail::AtomicIntegerT<int64_t> AtomicInt64;

} // namespace dwater

#endif // DWATER_SRC_BASE_ATOMIC_H
