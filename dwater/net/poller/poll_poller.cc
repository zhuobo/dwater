// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.24
// Filename:        poll_poller.cc
// Descripton:       

#include "dwater/net/poller/poll_poller.h"

#include "dwater/base/logging.h"
#include "dwater/base/types.h"
#include "dwater/net/channel.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>

using namespace dwater;
using namespace dwater::net;

PollPoller::PollPoller(EventLoop* loop) : Poller(loop) {}

PollPoller::~PollPoller() = default;

Timestamp PollPoller::Poll(int timeout_ms, ChannelList* active_channels) {
    int num_events = ::poll(&*pollfds_.begin(), pollfds_.size(), timeout_ms);
    int save_errno = errno;
    Timestamp now(Timestamp::Now());
    if ( num_events > 0 ) {
        LOG_TRACE << num_events << " events happened";
        FillActiveChannels(num_events, active_channels);
    } else if ( num_events == 0 ) {
        LOG_TRACE << " nothing happened";
    } else {
        if ( save_errno != EINTR ) { // 由于阻塞被终端导致无法得到，因此可以继续
            errno = save_errno;     //  其他情况就可能是出问题了
            LOG_SYSERR << "PollPoller::Poll()";
        }
    }
    return now;
}

// 时间复杂度O(n)
void PollPoller::FillActiveChannels(int num_events, ChannelList* active_channels) const {
    // 一共有发生的事件是num_events，因此如果能够提前找到所有发生事件
    // 的文件描述符，就不必继续找下去了
    for ( PollFdList::const_iterator it = pollfds_.begin();
            it != pollfds_.end() && num_events > 0; ++it ) {
        if ( it->revents > 0 ) {
            --num_events;
            ChannelMap::const_iterator ch = channels_.find(it->fd);
            assert(ch != channels_.end());
            Channel* channel = ch->second;
            assert(channel->Fd() == it->fd);
            channel->SetRevents(it->revents);
            active_channels->push_back(channel);
        }// 这个文件描述符有事件发生
    }
}

void PollPoller::UpdateChannel(Channel* channel) {
    Poller::AssertInLoopThread();
    LOG_TRACE << "fd = " << channel->Fd() << " events = " << channel->Events();
    if ( channel->Index() < 0 ) {
        // 确认原来是不存在这个Channel的
        assert(channels_.find(channel->Fd())  == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->Fd();
        pfd.events = static_cast<short>(channel->Events());
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        int index = static_cast<int>(pollfds_.size()) - 1;
        channel->SetIndex(index);
        channels_[pfd.fd] = channel;
    } else {
        assert(channels_.find(channel->Fd()) != channels_.end());
        assert(channels_[channel->Fd()] == channel);
        int index = channel->Index();
        assert(index >= 0 && index < static_cast<int>(pollfds_.size()));
        struct pollfd& pfd = pollfds_[index];
        assert(pfd.fd == channel->Fd() ||  pfd.fd == -channel->Fd() - 1);
        pfd.fd = channel->Fd();
        pfd.events = static_cast<short>(channel->Events());
        pfd.revents = 0;
        if ( channel->IsNoneEvent() ) {
            pfd.fd = -channel->Fd() - 1;
        }
    }
}

void PollPoller::RemoveChannel(Channel* channel) {
    Poller::AssertInLoopThread();
    LOG_TRACE << "fd = " << channel->Fd();
    assert(channels_.find(channel->Fd()) != channels_.end());
    assert(channels_[channel->Fd()] == channel);
    assert(channel->IsNoneEvent());
    int index = channel->Index();
    assert( index >= 0 && index < static_cast<int>(pollfds_.size()) );
    const struct pollfd& pfd = pollfds_[index];
    (void) pfd;
    assert(pfd.fd == -channel->Fd() - 1&& pfd.events == channel->Events());
    size_t n = channels_.erase(channel->Fd());
    assert(n == 1); (void)n;
    if ( implicit_cast<size_t>(index) == pollfds_.size() - 1 ) {
        pollfds_.pop_back();
    } else {
        int channel_an_end = pollfds_.back().fd;
        iter_swap(pollfds_.begin() + index, pollfds_.end() - 1);
        if ( channel_an_end < 0 ) {
            channel_an_end = -channel_an_end - 1;
        }
        channels_[channel_an_end]->SetIndex(index);
        pollfds_.pop_back();
    }
}

