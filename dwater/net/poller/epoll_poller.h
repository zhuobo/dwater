// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.24
// Filename:        epoll_poller.h
// Descripton:       

#ifndef DWATER_NET_POLLER_EPOLL_POLLER_H
#define DWATER_NET_POLLER_EPOLL_POLLER_H

#include "dwater/net/poller.h"

#include <vector>

struct epoll_event;

namespace dwater {
namespace net {

class EpollPoller : public Poller {
public:
    EpollPoller(EventLoop* loop);
    ~EpollPoller() override;

    Timestamp Poll(int time_out_ms, ChannelList *active_channels) override;
    void UpdateChannel(Channel* channel) override;
    void RemoveChannel(Channel* channel) override;
private:
    static const int kinit_event_list_size = 16;

    static const char* OperationToString(int op);

    void FillActiveChannels(int num_events, ChannelList* active_channels) const;

    void Update(int operation, Channel* channel);

    typedef std::vector<struct epoll_event> EventList;

    int epollfd_;
    EventList events_;
};
}
}

#endif 


