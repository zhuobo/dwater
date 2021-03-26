// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        timer.h
// Descripton:       


#ifndef DWATER_NET_TIMER_H
#define DWATER_NET_TIMER_H

#include "dwater/base/timestamp.h"
#include "dwater/base/atomic.h"
#include "dwater/net/callbacks.h"

namespace dwater {

namespace net {

/// 
/// 定时器
/// 定时器事件的一个内部类，封装了定时器的一些参数，比如超时回调函数、是否超时
/// 定时器是否重复，重复事件间隔等等
/// 
class Timer : noncopyable {
public:
    Timer(TimerCallback cb, Timestamp when, double interval)
        : callback_(std::move(cb)),
          expiration_(when),
          interval_(interval),
          repeat_(interval_ > 0.0), 
          sequence_(s_num_created_.IncrementAndGet()) {}

    void Run() const {
        callback_();
    }

    Timestamp Expiration() const {
        return expiration_;
    }

    bool Repeat() const {
        return repeat_;
    }

    int64_t Sequence() const {
        return sequence_;
    }

    void Restart(Timestamp now);

    static int64_t NumCreate() {
        return s_num_created_.Get();
    }
private:
    const TimerCallback callback_;
    Timestamp           expiration_;
    const double        interval_;
    const bool          repeat_;
    const int64_t       sequence_; // 序列号

    static AtomicInt64  s_num_created_; // 用来产生Timer创建的编号，必须要线程安全的
}; // class Timer

} // namespace net

} // namespace dwater 


#endif // DWATER_NET_TIMER_H



