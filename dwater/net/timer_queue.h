// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.23
// Filename:        timer_queue.h
// Descripton:  


#ifndef DWATER_NET_TIMER_QUEUE_H
#define DWATER_NET_TIMER_QUEUE_H

#include "dwater/base/mutex.h"
#include "dwater/base/timestamp.h"
#include "dwater/net/callbacks.h"
#include "dwater/net/channel.h"

#include <vector>
#include <set>

namespace dwater {

namespace net {

class EventLoop;
class Timer;
class TimerId;

/// 
/// 定时器队列
/// 定时器处理流程的封装
/// 
class TimerQueue : noncopyable {
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    ///
    /// 加入一个定时器任务，如果 @c interval > 0.0 就重复执行这个任务
    /// 
    /// 必须要保证这个函数是线程安全的，因为别的线程也会调用这个函数加入定时器任务
    TimerId AddTimer(TimerCallback cb, Timestamp  when, double interval);

    void Cancel(TimerId timer_id);

private:
    typedef std::pair<Timestamp, Timer*>    Entry;
    typedef std::set<Entry>                 TimerList;
    typedef std::pair<Timer*, int64_t>      ActiveTimer;
    typedef std::set<ActiveTimer>           ActiveTimerSet;

    void AddTimerInLoop(Timer* timer);
    void CancelInLoop(TimerId timer_id);
    void HandleRead();

    std::vector<Entry> GetExpired(Timestamp now);
    void Reset(const std::vector<Entry>& expired, Timestamp now);

    ///
    /// @brief 往活动的定时器表中插入一个定时器
    /// @param timer 要插入的定时器
    /// @return 是否成功插入定时器
    /// 
    bool Insert(Timer* timer);

    EventLoop*      loop_;
    const int       timerfd_;
    Channel         timerfd_channel_;
    TimerList       timers_;
    ActiveTimerSet  active_timers_;
    bool            calling_expired_timers_;
    ActiveTimerSet  canceling_timers_;
}; // class TimerQueue

} // namespace net

} // namespace 


#endif // DWATER_NET_TIMER_QUEUE_H



