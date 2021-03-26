// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.22
// Filename:        channel.cc
// Descripton:      

#include "dwater/net/channel.h"

#include "dwater/base/logging.h"
#include "dwater/net/event_loop.h"

#include <sstream>
#include <poll.h>

using namespace dwater;
using namespace dwater::net;

const int Channel::knone_event = 0;
const int Channel::kread_event = POLLIN | POLLPRI; // readable
const int Channel::kwrite_event = POLLOUT; // write

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      revents_(0),
      index_(-1),
      log_hup_(true),
      tied_(false),
      event_handling_(false),
      added_to_loop_(false) {}

Channel::~Channel() {
  assert(!event_handling_);
  assert(!added_to_loop_);
  if ( loop_->IsInLoopThread() ) {
      assert(!loop_->HasChannel(this));
  }
}


// 将Channel绑定到Channel的所有者
void Channel::Tie(const std::shared_ptr<void>& obj) {
    tie_ = obj;
    tied_ = true;
}

void Channel::Update() {
    added_to_loop_ = true;
    loop_->UpdateChannel(this);
}

void Channel::Remove() {
    assert(IsNoneEvent());
    added_to_loop_ = false;
    loop_->RemoveChannel(this);
}

void Channel::HandleEvent(Timestamp receive_time) {
    std::shared_ptr<void> guard;
    if ( tied_ ) {
        guard = tie_.lock();
        if ( guard ) {
            HandleEventWithGuard(receive_time);
        }
    } else {
        HandleEventWithGuard(receive_time);
    }
}

void Channel::HandleEventWithGuard(Timestamp receive_time) {
    event_handling_ = true;
    LOG_TRACE << ReventsToString();
    if ( (revents_ & POLLHUP) && !(revents_ & POLLIN) ) {
        if ( log_hup_ ) {
            LOG_WARN << "fd = " << fd_ << " Channel::HandleEvent() POLLHUP";
        }
        if ( close_callback_ ) {
            error_callback_();
        }
    }

    if ( revents_ & POLLNVAL ) {
        LOG_WARN << "fd = " << fd_ << " Channel::HandleEvent() POLLNVAL";
    }

    if ( revents_ & (POLLERR | POLLNVAL) ) {
        if ( error_callback_ ) error_callback_();
    }

    if ( revents_ & (POLLIN | POLLPRI | POLLRDHUP) )  {
        if ( read_callback_ )  read_callback_(receive_time);
    }

    if ( revents_ & POLLOUT ) {
        if ( write_callback_ ) write_callback_();
    }
    event_handling_ = false;
}

string Channel::ReventsToString() const {
    return EventsToString(fd_, revents_);
}

string Channel::EventsToString() const {
    return EventsToString(fd_, events_);
}

string Channel::EventsToString(int fd, int events) {
    std::ostringstream oss;
    oss << fd << ": ";
    if ( events & POLLIN ) oss << "IN ";
    if ( events & POLLPRI ) oss << "PRI ";
    if ( events & POLLOUT ) oss << "OUT ";
    if ( events & POLLHUP ) oss << "HUP ";
    if ( events & POLLRDHUP ) oss << "RDHUP ";
    if ( events & POLLERR ) oss << "ERR ";
    if ( events & POLLNVAL ) oss << "NVAL ";
    return oss.str();
}
