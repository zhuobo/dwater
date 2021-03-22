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

class Poller :  noncopyable {
public:
    typedef std::vector<Channel*> ChannelList;

    Poller(EventLoop* loop);

    virtual ~Poller();

    // 轮询这些事件
    virtual Timestamp Poll(int time_out_ms, ChannelList* active_channels) = 0;

    // 更新感兴趣的IO事件
    virtual void UpdateChannel(Channel* channel) = 0;

    // 删除IO事件
    virtual void RemoveChannel(Channel* channel) = 0;

    // 判断是否拥有某个IO事件
    virtual  bool HasChannel(Channel* channel) const;

    static Poller* NewDefaultPoller(EventLoop* loop);

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

