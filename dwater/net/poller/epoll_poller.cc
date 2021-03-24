// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.24
// Filename:        epoll_poller.cc
// Descripton:       


#include "dwater/net/poller/epoll_poller.h"
#include "dwater/base/logging.h"
#include "dwater/net/channel.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

using namespace dwater;
using namespace dwater::net;

static_assert(EPOLLIN == POLLIN,        "epoll uses same flag value poll");
static_assert(EPOLLPRI == POLLPRI,      "epoll uses same flag value poll");
static_assert(EPOLLOUT == POLLOUT,      "epoll uses same flag value poll");
static_assert(EPOLLRDHUP == POLLRDHUP,  "epoll uses same flag value poll");
static_assert(EPOLLERR == POLLERR,      "epoll uses same flag value poll");
static_assert(EPOLLHUP == POLLHUP,      "epoll uses same flag value poll");

namespace {
    const int knew = -1;
    const int kadded = 1;
    const int kdelete = 2;
}

EpollPoller::EpollPoller(EventLoop* loop) : Poller(loop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kinit_event_list_size) {
    if ( epollfd_ < 0 ) {
        LOG_SYSFATAL << "EpollPoller::EpollPoller";
    }
}

EpollPoller::~EpollPoller() {
    ::close(epollfd_);
}

Timestamp EpollPoller::Poll(int timeout_ms, ChannelList* active_channels) {
    LOG_TRACE << "fd total count is " << channels_.size();
    int num_events = ::epoll_wait(epollfd_,
                                  &*events_.begin(),
                                  static_cast<int>(events_.size()), 
                                  timeout_ms);
    int saved_errno = errno;
    Timestamp now(Timestamp::Now());
    if ( num_events > 0 ) {
        LOG_TRACE << num_events << " events happened";
        FillActiveChannels(num_events, active_channels);
        if ( implicit_cast<size_t>(num_events) == events_.size() ) {
            events_.resize(events_.size() * 2);
        }
    } else if ( num_events == 0 ) {
        LOG_TRACE << "nothing happened";
    } else {
        if ( saved_errno != EINTR ) {
            errno = saved_errno;
            LOG_SYSERR << "EpollPoller::Poll()";
        }
    }
    return now;
}

void EpollPoller::FillActiveChannels(int num_events, ChannelList* active_channels) const {
    assert(implicit_cast<size_t>(num_events) <= events_.size());
    for ( int i = 0; i < num_events; ++i ) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
#ifndef NDEBUG
        int fd = channel->Fd();
        ChannelMap::const_iterator it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == channel);
#endif
        channel->SetRevents(events_[i].events);
        active_channels->push_back(channel);
    }
}

void EpollPoller::UpdateChannel(Channel* channel) {
    Poller::AssertInLoopThread();
    const int index = channel->Index();
    LOG_TRACE << "fd = " << channel->Fd() << " events = " << channel->Events()
              << " index = " << index;
    if ( index == knew || index == kdelete ) {
        int fd = channel->Fd();
        if ( index == knew ) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        } else {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] = channel);
        }
        channel->SetIndex(kadded);
        Update(EPOLL_CTL_ADD, channel);
    } else {
        int fd = channel->Fd();
        (void)fd;
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kadded);
        if ( channel->IsNoneEvent() ) {
            Update(EPOLL_CTL_DEL, channel);
            channel->SetIndex(kdelete);
        } else {
            Update(EPOLL_CTL_MOD, channel);
        }
    }
}


void EpollPoller::RemoveChannel(Channel* channel) {
    Poller::AssertInLoopThread();
    int fd = channel->Fd();
    LOG_TRACE << "fd = " << fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->IsNoneEvent());
    int index = channel->Index();
    assert(index == kadded || index == kdelete);
    size_t n = channels_.erase(fd); (void)n;
    assert(n == 1);

    if ( index == kadded ) {
        Update(EPOLL_CTL_DEL, channel);
    }
    channel->SetIndex(knew);
}

void EpollPoller::Update(int operation, Channel* channel) {
    struct epoll_event event;
    MemZero(&event, sizeof(event));
    event.events = channel->Events();
    event.data.ptr = channel;
    int fd  = channel->Fd();
    LOG_TRACE << "epoll_ctl op = " << OperationToString(operation)
              << "  fd = " << fd << " event = { " << channel->EventsToString() << " }";
    if (  ::epoll_ctl(epollfd_, operation, fd, &event) < 0 ) {
        if ( operation == EPOLL_CTL_DEL ) {
            LOG_SYSERR << "epoll_ctl op = " << OperationToString(operation) << " fd = " << fd;
        } else {
            LOG_SYSFATAL << "epoll_ctl op = " <<  OperationToString(operation) << " fd = " << fd;
        }
    }

}

const char* EpollPoller::OperationToString(int op) {
    switch(op) {
    case EPOLL_CTL_ADD:
        return "ADD";
    case EPOLL_CTL_DEL:
        return "DEL";
    case EPOLL_CTL_MOD:
        return "MOD";
    default:
        assert(false && "ERROR op");
        return "Unknown Operation";
    }
}
