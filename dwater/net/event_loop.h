// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.21
// Filename:        event_loop.h
// Descripton:      事件循环，reactor.每个线程只有一个LoopEvent对象，LoopEvent对象
// 会记住自己所属于的线程, EventLoop构造函数会记住自己所属于的线程，主要功能能就是
// 运行事件循环Loop(), 生命周期和线程一样长，不必是head对象

#ifndef DWATER_NET_EVENT_LOOP_H
#define DWATER_NET_EVENT_LOOP_H

#include "dwater/base/mutex.h"
#include "dwater/base/current_thread.h"
#include "dwater/base/timestamp.h"
#include "dwater/net/callbacks.h"
#include "dwater/net/timerid.h"

#include <boost/any.hpp>

#include <atomic>
#include <functional>
#include <vector>

namespace dwater {

namespace net {

// forward declaretion
class Channel;          // 每个socket连接的事件分发
class Poller;           // IO复用的基类借口
class TimerQueue;       // 定时器队列

class EventLoop : noncopyable {
public:
    typedef std::function<void()> Functor;
    ///
    /// @brief 一个线程只能有一个LoopEvent对象，构造函数在构造对象的时候检查当前线程
    ///         是否已经有其他的LoopEvent对象了，如已经有就终止程序LOG_FATAL
    /// 
    EventLoop(); 

    ~EventLoop();

    /// @brief 开始事件循环
    ///
    /// 通过poll,epoll阻塞等待一个或多个文件描述符上的就绪事件，然后根据返回的事
    /// 调用相应的回调函数处理
    ///
    void Loop(); // start loop

    /// @brief 被别的线程调用，设置循环条件，一下轮停止循环
    ///
    void Quit(); // quit loop

    /// @return 上一次poller的返回时间
    Timestamp PollReturnTime() const {
        return poll_return_time_;
    }

    /// @return poller循环轮数
    int64_t Iteration() const {
        return iteration_;
    }

    /// 
    /// @brief 尝试执行回调函数，如果不是EventLoop所在线程，那么就加入队列中
    /// @proma cb 要添加的回调函数
    /// 
    void RunInLoop(Functor cb); // run callback in the loop thread immediately

    /// 
    /// @brief 添加回调函数到EventLoop的回调函数队列
    /// @prama cb 要添加的回调函数
    /// 
    /// 如果此时不是在当前线程执行，那么就将回调函数加入到队列中，之后
    /// 再执行
    void QueueInLoop(Functor cb); // queues callback in the loop thread

    /// 
    /// @brief 回调函数队列大小
    /// 
    size_t QueueSize() const; // callback queue size

    /// 
    /// @brief 添加定时器事件，在某个具体的时间执行
    /// @prama time 回调函数执行的时间
    /// @prama cb   定时器的回调函数
    /// @return 定时器的Id
    /// 
    TimerId RunAt(Timestamp time, TimerCallback cb); // run callback at time

    /// 
    /// @brief 添加定时器事件，在某个时间后执行
    /// @prama delay 间隔delay秒后
    /// @prama cb   定时器的回调函数
    /// @return 定时器的Id
    /// 
    TimerId RunAfter(double delay, TimerCallback cb); // run callback after delay secondes

    /// 
    /// @brief 添加定时器事件，循环事件
    /// @prama interval 循环周期
    /// @prama cb   定时器的回调函数
    /// @return 定时器的Id
    /// 
    TimerId RunEvery(double interval, TimerCallback cb); // run every interval seconds

    /// 
    /// @brief 取消定时事件
    /// @prama timer_id 要取消的定时器id
    ///
    void Cancel(TimerId timer_id); // cancel timer

    /// 
    /// @brief 唤醒poller
    /// 
    /// 往wakeupfd_中写一个数字，wakeupfd_是一个eventfd, EventLoop注册了这个事件
    /// 的回调函数，当前线程被唤醒，唤醒阻塞的线程，其他线程主动唤醒继续执行之后
    /// 的回调队列的函数
    /// 
    void Wakeup();
    void UpdateChannel(Channel*);
    void RemoveChannel(Channel*);
    bool HasChannel(Channel*);

    void AssertInLoopThread() {
        if ( !IsInLoopThread() ) {
            AbortNotInLoopThread();
        }
    }

    bool IsInLoopThread() const {
        return thread_id_ == current_thread::Tid();
    }

    bool EventHandling() const {
        return event_handling_;
    }

    void SetContext(const boost::any& context) {
        context_ = context;
    }

    const boost::any& GetContet() const {
        return context_;
    }

    boost::any* GetMutableContet() {
        return &context_;
    }

    // 每个线程只有一个EventLoop对象，返回这个对象
    static EventLoop* GetEventLoopOfCurrentThead();

private:
    void AbortNotInLoopThread(); // 
    void HandleRead();
    ///
    /// @brief 执行回调函数队列中的回调函数
    /// 
    /// 一个线程一个EventLoop，但是一个EventLoop可能被不同的线程执行，如果不是当前
    /// 线程，那么回调函数不应该被执行，于是加入到一个回调函数队列中，等待当前线程
    /// 执行
    /// 
    void DoPendingFunctors();

    ///  @brief 打印本次就绪的Channel的对应的事件，同时也调用了Channel的ToString函数
    ///
    void PrintActiveChannels() const;

    typedef std::vector<Channel*> ChannelList;

    bool                        looping_; // atomic
    std::atomic<bool>           quit_;
    bool                        event_handling_; // atomic
    bool                        calling_pending_functors_; // atomic
    int64_t                     iteration_; // poller被唤醒的次数
    const pid_t                 thread_id_; // 当前EventLoop所属于线程，一个线程一个对象
    Timestamp                   poll_return_time_; // poller上一次被唤醒的时间
    std::unique_ptr<Poller>     poller_; // poller
    std::unique_ptr<TimerQueue> timer_queue_; // 定时器队列
    int                         wakeup_fd_; // IO线程关注这个事件，其他线程如果要通知IO线程执行回调，就往这里写8个字节的数据

    std::unique_ptr<Channel>    wakeup_channel_; // 处理wakeup_上的readable事件
    boost::any                  context_;

    ChannelList                 active_channels_; // 就绪的Channel
    Channel*                    curr_active_channel_; // 当前正在处理的Channel

    mutable MutexLock           mutex_;
    std::vector<Functor>        pending_functors_ GUARDED_BY(mutex_); // EventLoop需要执行的函数对象
}; // class EventLoop

} // namespace net

} // namespace dwater
#endif // DWATER_SRC_NET_EVENT_LOOP_H
