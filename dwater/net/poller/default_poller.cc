// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.23
// Filename:        default_poller.cc
// Descripton:      

#include "dwater/net/poller.h"
#include "dwater/net/poller/epoll_poller.h"
#include "dwater/net/poller/poll_poller.h"

#include <stdlib.h>

using namespace dwater::net;

Poller* Poller::NewDefaultPoller(EventLoop* loop) {
    if ( getenv("DWATER_USE_POLL") ) {
        return new PollPoller(loop);
    } else {
        return new EpollPoller(loop);
    }
}



