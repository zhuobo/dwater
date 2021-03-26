// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.23
// Filename:        poll_poller.h
// Descripton:      poller with poll


#ifndef DWATER_NET_POLLER_POLL_POLLER_H
#define DWATER_NET_POLLER_POLL_POLLER_H

#include "dwater/net/poller.h"

#include <vector>

struct pollfd;

namespace dwater {

namespace net {

///
/// 用poll实现的poller
/// 
class PollPoller : public Poller {
public:
    PollPoller(EventLoop* loop);

    ~PollPoller() override;

    Timestamp Poll(int time_out_ms, ChannelList *active_channels) override;

    ///
    /// 更新维护pollfds_数组，插入或者更新已有的
    ///
    void UpdateChannel(Channel* channel) override;

    /// 
    /// 删除一个已有的Channel
    /// 
    void RemoveChannel(Channel* channel) override;

private:
    ///
    /// @brief 将发生的事件插入到active_channels中
    /// @prama num_events poll()返回的发生事件数量
    /// @prama active_channels 活动Channel数组
    /// 
    void FillActiveChannels(int num_events, ChannelList* active_channels) const;

    typedef std::vector<struct pollfd> PollFdList;

    PollFdList pollfds_;
}; // class PollPoller


} // namesapce net

} // namespace dwater

#endif // DWATER_NET_POLLER_POLL_POLLER_H 
