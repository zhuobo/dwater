// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.25
// Filename:        tcp_connection.cc
// Descripton:       

#include "dwater/net/tcp_connection.h"

#include "dwater/base/logging.h"
#include "dwater/base/weak_callback.h"
#include "dwater/net/event_loop.h"
#include "dwater/net/channel.h"
#include "dwater/net/socket.h"
#include "dwater/net/socket_ops.h"

#include <errno.h>

using namespace dwater;
using namespace dwater::net;

void dwater::net::DefaultConnectionCallback(const TcpConnectionPtr& conn) {
    LOG_TRACE << conn->LocalAddress().ToIpPort() << " -> "
              << conn->PeerAddress().ToIpPort() << " is "
              << (conn->Connected() ? "UP" : "DOWN");
}

void dwater::net::DefaultMessageCallback(const TcpConnectionPtr&,
        Buffer* buf,
        Timestamp) {
    buf->RetrieveAll();
}

TcpConnection::TcpConnection(EventLoop* loop,
                             const string& name_arg,
                             int sockfd,
                             const InetAddress& local_addr,
                             const InetAddress& peer_addr)
    : loop_(CHECK_NOTNULL(loop)),
      name_(name_arg),
      state_(kconnecting),
      reading_(true),
      socket_(new Socket(sockfd)),
      local_addr_(local_addr),
      peer_addr_(peer_addr),
      high_water_mark_(64 * 1024 * 1024) {
    channel_->SetReadCallback(std::bind(&TcpConnection::HandleRead, this, _1));
    channel_->SetWriteCallback(std::bind(&TcpConnection::HandleWrite, this));
    channel_->SetCloseCallback(std::bind(&TcpConnection::HandleClose, this));
    channel_->SetErrorCallback(std::bind(&TcpConnection::HandleError, this));
    LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at " << this << " fd = " <<  sockfd;
    socket_->SetKeepAlive(true);
}

TcpConnection::~TcpConnection() {
    LOG_DEBUG << "TcpConnection::dtor[" << name_ << "] at " << this << " fd = "
              << channel_->Fd() << " state = " << StateToString();
    assert(state_ == kdisconnected);
}

bool TcpConnection::GetTcpInfo(struct tcp_info* tcpi) const {
    return socket_->GetTcpInfo(tcpi);
}

string TcpConnection::GetTcpInfoString() const {
    char buf[1024];
    buf[0] = '\0';
    socket_->GetTcpInfoString(buf, sizeof(buf));
    return buf;
}

void TcpConnection::Send(const void* data, int len) {
    Send(StringPiece(static_cast<const char*>(data), len));

}

void TcpConnection::Send(const StringPiece& message) {
    if ( state_ == kconnected ) {
        if ( loop_->IsInLoopThread() ) {
            SendInLoop(message);
        } else {
            void (TcpConnection::*fp)(const StringPiece& message) = &TcpConnection::SendInLoop;
            loop_->RunInLoop(std::bind(fp,
                                       this,
                                       message.AsString()));
        }
    }
}


void TcpConnection::Send(Buffer* buf) {
    if ( state_ == kconnected ) {
        if ( loop_->IsInLoopThread() ) {
            SendInLoop(buf->Peek(), buf->ReadableBytes());
            buf->RetrieveAll();
        } else {
            void (TcpConnection::*fp)(const StringPiece& message) = &TcpConnection::SendInLoop;
            loop_->RunInLoop(std::bind(fp,
                                       this,
                                       buf->RetrieveAllAsString()));
        }
    }
}

void TcpConnection::SendInLoop(const StringPiece&  message) {
    SendInLoop(message.Data(), message.Size());
}

void TcpConnection::SendInLoop(const void* data, size_t len) {
    loop_->AssertInLoopThread();
    ssize_t n_wrote = 0;
    size_t remaining = len;
    bool fault_error = false;
    if ( state_ == kdisconnected ) {
        LOG_WARN << "disconnected, give up writing";
        return;
    }
    if ( !channel_->IsWriting() && output_buffer_.ReadableBytes() == 0 ) {
        n_wrote = socket::Write(channel_->Fd(), data, len);
        if ( n_wrote >= 0 ) {
            remaining = len - n_wrote;
            if ( remaining == 0 && write_complete_callback_ ) {
                loop_->QueueInLoop(std::bind(write_complete_callback_, shared_from_this()));
            }
        } else {
            n_wrote = 0;
            if ( errno != EWOULDBLOCK ) {
                LOG_SYSERR << "TcpConnection::SendInLoop";
                if ( errno == EPIPE || errno == ECONNRESET ) {
                    fault_error = true;
                }
            }
        }
    }

    assert(remaining <= len);
    if ( !fault_error && remaining > 0 ) {
        size_t old_len = output_buffer_.ReadableBytes();
        // 超过高水位，就触发高水位回调发送数据
        if ( old_len + remaining >= high_water_mark_
            && old_len < high_water_mark_
            && high_water_mark_callback_) {
            loop_->QueueInLoop(std::bind(high_water_mark_callback_, shared_from_this(), old_len + remaining));
        }
        output_buffer_.Append(static_cast<const char*>(data) + n_wrote, remaining);
        if ( !channel_->IsWriting() ) {
            channel_->EnableWriting();
        }
    }
}

void TcpConnection::Shutdown() {
    if ( state_ == kconnected ) {
        SetState(kdisconnecting);
        loop_->RunInLoop(std::bind(&TcpConnection::ShutdownInLoop, this));
    }
}

void TcpConnection::ShutdownInLoop() {
    loop_->AssertInLoopThread();
    if ( !channel_->IsWriting() ) {
        socket_->ShutdownWrite();
    }
}

void TcpConnection::ForceClose() {
    if ( state_ == kconnected || state_ == kdisconnecting ) {
        SetState(kdisconnecting);
        loop_->QueueInLoop(std::bind(&TcpConnection::ForceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::ForceCloseWithDelay(double seconds) {
    if ( state_ == kconnected || state_ == kdisconnecting ) {
        SetState(kdisconnecting);
        loop_->RunAfter(
                seconds,
                MakeWeakCallback(shared_from_this(), &TcpConnection::ForceClose));
    }
}

void TcpConnection::ForceCloseInLoop() {
    loop_->AssertInLoopThread();
    if ( state_ == kconnected ||  state_ == kdisconnecting ) {
        HandleClose();
    }
}

const char* TcpConnection::StateToString() const {
    switch (state_) {
    case kdisconnected:
        return "kdisconnected";
    case kconnecting:
        return "kconnecting";
    case kdisconnecting:
        return "kdisconnecting";
    case kconnected:
        return "kconnected";
    default:
        return "unknown state";
    }
}

void TcpConnection::SetTcpNoDelay(bool on) {
    socket_->SetTcpNoDelay(on);
}

void TcpConnection::StartRead() {
    loop_->RunInLoop(std::bind(&TcpConnection::StartReadInLoop, this));
}

void TcpConnection::StartReadInLoop() {
    loop_->AssertInLoopThread();
    if ( !reading_ || !channel_->IsReading() ) {
        channel_->EnableReading();
        reading_ = true;
    }
}

void TcpConnection::StopRead() {
    loop_->RunInLoop(std::bind(&TcpConnection::StopReadInLoop, this));
}

void TcpConnection::StopReadInLoop() {
    loop_->AssertInLoopThread();
    if ( reading_ || channel_->IsReading() ) {
        channel_->DisableReading();
        reading_ = false;
    }
}

void TcpConnection::ConnectionEstablished() {
    loop_->AssertInLoopThread();
    assert(state_ == kconnecting);
    SetState(kconnected);
    channel_->Tie(shared_from_this());
    channel_->EnableReading();
    connection_callback_(shared_from_this());
}

void TcpConnection::ConnectionDestroyed() {
    loop_->AssertInLoopThread();
    if ( state_ == kconnected ) {
        SetState(kdisconnected);
        channel_->DisableAll();
        connection_callback_(shared_from_this());
    }
    // 主动移除自己
    channel_->Remove();
}

void TcpConnection::HandleRead(Timestamp receive_time) {
    loop_->AssertInLoopThread();
    int saved_errno = 0;
    ssize_t n = input_buffer_.ReadFd(channel_->Fd(), &saved_errno);
    if ( n > 0 ) {
        message_callback_(shared_from_this(), &input_buffer_, receive_time);
    } else if (n == 0) {
        HandleClose();
    }else {
        errno = saved_errno;
        LOG_SYSERR << "TcpConnection::HandleRead";
        HandleError();
    }
}

void TcpConnection::HandleWrite() {
    loop_->AssertInLoopThread();
    if ( channel_->IsWriting() ) {
        ssize_t n = socket::Write(channel_->Fd(),
                                  output_buffer_.Peek(),
                                  output_buffer_.ReadableBytes());
        if ( n > 0 ) {
            output_buffer_.Retrieve(n);
            if ( output_buffer_.ReadableBytes() == 0 ) {
                channel_->DisableWriting();
                if ( write_complete_callback_ ) {
                    loop_->QueueInLoop(std::bind(write_complete_callback_, shared_from_this()));
                }
                if ( state_ == kdisconnecting ) {
                    ShutdownInLoop();
                }
            }
        } else {
            LOG_SYSERR << "TcpConnection::HandleWrite";
        }
    } else {
        LOG_TRACE << "Connection fd = " << channel_->Fd() << "  is down, no more writing";
    }
}

void TcpConnection::HandleClose() {
    loop_->AssertInLoopThread();
    LOG_TRACE << "fd = " << channel_->Fd() << " state = " << StateToString();
    assert(state_ == kconnected || state_ == kdisconnecting);
    SetState(kdisconnected);
    channel_->DisableAll();
    
    TcpConnectionPtr guard_this(shared_from_this());
    connection_callback_(guard_this);
    close_callback_(guard_this);
}

void TcpConnection::HandleError() {
    int err = socket::GetSocketError(channel_->Fd());
    LOG_ERROR << "TcpConnection::HandleError [" << name_ << "] - SO_ERROR"
              << err << " " << strerror_tl(err);
}
