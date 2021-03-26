// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.26
// Filename:        event_loop_thread_pool.h
// Descripton:       

#ifndef DWATER_NET_EVENT_LOOP_THREAD_POOL_H
#define DWATER_NET_EVENT_LOOP_THREAD_POOL_H

#include "dwater/base/noncopable.h"
#include "dwater/base/types.h"

#include <memory>
#include <vector>
#include <functional>

namespace dwater {

namespace net {

class EventLoop;
class EventLoopThread;

///
/// EventLoopThread的简单线程池
///
class EventLoopThreadPool : noncopyable {
public:
    typedef std::function<void (EventLoop*)> ThreadInitCallback;

    EventLoopThreadPool(EventLoop* base_loop, const string& name);

    ///
    /// 什么都不做，线程池是一个栈上的对象不会被释放
    /// 
    ~EventLoopThreadPool();

    ///
    /// 设置线程池中线程的数量
    /// 
    void SetThreadNum(int num_thread) { num_thread_ = num_thread; }

    ///
    /// 创建num_thread_个EventLoopThread线程存储起来
    ///
    void Start(const ThreadInitCallback& cb = ThreadInitCallback());

    ///
    /// 获取下一个线程，使用round-robin算法，返回线程里的EventLoop，没有了从0开始获取
    /// 
    EventLoop* GetNextLoop();

    
    ///
    /// 根据哈希值获取一个线程，返回线程里的EventLoop，哈希值对长度取模
    /// 
    EventLoop* GetLoopForHash(size_t hash_code);

    std::vector<EventLoop*> GetAllLoops();

    bool Started() const { return started_; }

    const string& Name() const { return name_; }
private:
    EventLoop*      base_loop_;
    string          name_;
    bool            started_;
    int             num_thread_;
    int             next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};

} // namespace net
} // namespace dwater
#endif
