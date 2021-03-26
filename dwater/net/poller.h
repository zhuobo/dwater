// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.22
// Filename:        poller.h
// Descripton:      封装底层系统调用poll以及epoll,是IO复用的基类,Channel不会是
// poller的成员,poller被IO复用集成，用来实现两种不同的IO复用poll以及epoll

#ifndef DWATER_NET_POLLER_H
#define DWATER_NET_POLLER_H

#include "dwater/base/timestamp.h"
#include "dwater/net/event_loop.h"

#include <vector>
#include <map>

namespace dwater {

namespace net {

class Channel;

/// 
/// @brief pollepoll两种IO复用的基类
///
/// Poller的核心在于函数Poll(), 是一个纯虚函数，调用IO复用方法获得当前活动的IO
/// 事件，然后填充调用放传入的active_channels
class Poller :  noncopyable {
public:
    typedef std::vector<Channel*> ChannelList;

    Poller(EventLoop* loop);

    virtual ~Poller();

    /// @brief 调用IO复用函数获得当前活动的IO事件
    /// @param time_out_ms 超时时间
    /// @param active_channels 调用方传入的活动事件的集合，被本函数填充
    /// @return poll return的时间
    virtual Timestamp Poll(int time_out_ms, ChannelList* active_channels) = 0;

    /// @brief 更新channels_，添加新的channel
    /// @parma channel 新加入的Channel
    ///
    /// 更新已有的Channel时间复杂度为O(1),插入新的Channel时间复杂度为O(logN)
    virtual void UpdateChannel(Channel* channel) = 0;

    /// @brief 删除一个Channel
    /// @parma channel 要移除的Channel
    virtual void RemoveChannel(Channel* channel) = 0;

    /// 判断是否拥有某个IO事件
    virtual  bool HasChannel(Channel* channel) const;

    static Poller* NewDefaultPoller(EventLoop* loop);

    /// 是否在IO线程执行
    void AssertInLoopThread() const {
        ower_loop_->AssertInLoopThread();
    }
protected:
    typedef std::map<int, Channel*> ChannelMap;
    ChannelMap channels_; // 当前poller关注的事件，fd->Channel

private:
    EventLoop* ower_loop_;
}; // class Poller


} // net

} // dwater


#endif //DWATER_NET_POLLER_H

