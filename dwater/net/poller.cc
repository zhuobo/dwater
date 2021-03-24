// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.22
// Filename:        poller.cc
// Descripton:      

#include "dwater/net/poller.h"
#include "dwater/net/channel.h"

using namespace dwater;
using namespace dwater::net;

Poller::Poller(EventLoop* loop) : ower_loop_(loop) {}

Poller::~Poller() = default;

bool Poller::HasChannel(Channel* channel) const {
    AssertInLoopThread();
    ChannelMap::const_iterator iter = channels_.find(channel->Fd());
    return iter != channels_.end() && iter->second == channel;
}

