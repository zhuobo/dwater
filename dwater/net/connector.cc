// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.26
// Filename:        connector.cc
// Descripton:       

#include "dwater/net/connector.h"

#include "dwater/base/logging.h"
#include "dwater/net/channel.h"
#include "dwater/net/event_loop.h"
#include "dwater/net/socket_ops.h"

#include <errno.h>

using namespace dwater;
using namespace dwater::net;

const int Connector::kmax_retry_delay_ms;

Connector::Connector(EventLoop* loop, const InetAddress& server_addr)
    : loop_(loop),
      server_addr_(server_addr),
      connect_(false),
      state_(kdisconnected),
      retry_delay_ms_(kinit_retry_delay_ms) {
          LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector() {
    LOG_DEBUG <<  "dtor[" << this << "]";
    assert(!channel_);
}

void Connector::Start() {
    connect_ = true;
    loop_->RunInLoop(std::bind(&Connector::StartInLoop, this));
}

void Connector::StartInLoop() {
    loop_->AssertInLoopThread();
    assert(state_ == kdisconnected);
    if ( connect_ ) {
        Connect();
    } else {
        LOG_DEBUG << "do not connect";
    }
}

void Connector::Stop() {
    connect_ = false;
    loop_->QueueInLoop(std::bind(&Connector::StopInLoop, this));
}

void Connector::StopInLoop() {
    loop_->AssertInLoopThread();
    if ( state_ == kconnecting ) {
        SetState(kdisconnected);
        int sockfd = RemoveAndResetChannel();
        Retry(sockfd);
    }
}

void Connector::Connect() {
    int sockfd = socket::CreateNonblockingOrDie(server_addr_.Family());
    int ret = socket::Connect(sockfd, server_addr_.GetSockAddr());
    int saved_errno = (ret == 0) ? 0 : errno;
    switch ( saved_errno ) {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        Connecting(sockfd);
        break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        Retry(sockfd);
        break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        LOG_SYSERR << "connect error in Connector::startInLoop " << saved_errno;
        socket::Close(sockfd);
        break;

    default:
        LOG_SYSERR << "Unexpected error in Connector::startInLoop " << saved_errno;
        socket::Close(sockfd);
        // connectErrorCallback_();
        break;
    }
}

void Connector::Restart() {
    loop_->AssertInLoopThread();
    SetState(kdisconnected);
    retry_delay_ms_ = kinit_retry_delay_ms;
    connect_ = true;
    StartInLoop();
}

void Connector::Connecting(int  sockfd) {
    SetState(kconnecting);
    assert(!channel_);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->SetWriteCallback(
            std::bind(&Connector::HandleWrite, this)
            );
    channel_->SetErrorCallback(
            std::bind(&Connector::HandleError, this)
            );
    channel_->EnableWriting();
}

int Connector::RemoveAndResetChannel() {
    channel_->DisableAll();
    channel_->Remove();
    int sockfd = channel_->Fd();
    loop_->QueueInLoop(std::bind(&Connector::ResetChannel, this));
    return sockfd;
}

void Connector::ResetChannel() {
    channel_.reset();
}

void Connector::HandleWrite() {
    LOG_TRACE << "Connector::HandleError " << state_;
    if ( state_ == kconnecting ) {
        int sockfd = RemoveAndResetChannel();
        int err = socket::GetSocketError(sockfd);
        if ( err ) {
            LOG_WARN << " Connector::HandleError - SO_ERROR = "
                     << err << " " << strerror_tl(err);
            Retry(sockfd);
        } else if ( socket::IsSelfConnect(sockfd) ) {
            LOG_WARN << "Connector::HandleWrite - Self connect";
            Retry(sockfd);
        } else  {
            SetState(kconnected);
            if ( connect_ ) {
                new_connection_callback_(sockfd);
            } else {
                socket::Close(sockfd);
            }
        }
    } else {
        assert(state_ == kdisconnected);
    }
}

void Connector::HandleError() {
    LOG_ERROR << "Connector::HandleError  state = " << state_;
    if ( state_ == kconnecting ) {
        int sockfd = RemoveAndResetChannel();
        int err = socket::GetSocketError(sockfd);
        LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
        Retry(sockfd);
    }
}

void Connector::Retry(int sockfd) {
    socket::Close(sockfd);
    SetState(kdisconnected);
    if ( connect_ ) {
        LOG_INFO << "Connector::Retry - Retry connecting  to "  << server_addr_.ToIpPort()
                 << " in " << retry_delay_ms_ << " milliseconds. ";
        loop_->RunAfter(retry_delay_ms_ / 1000.0,
                std::bind(&Connector::StartInLoop, shared_from_this()));
        retry_delay_ms_ = std::min(retry_delay_ms_ * 2, kmax_retry_delay_ms);
    } else {
        LOG_DEBUG << "do not connect";
    }
}
