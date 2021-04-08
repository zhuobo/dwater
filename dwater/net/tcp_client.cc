// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.04.08
// Filename:        tcp_client.cc
// Descripton:       

#include "dwater/net/tcp_client.h"

#include "dwater/base/logging.h"
#include "dwater/net/connector.h"
#include "dwater/net/event_loop.h"
#include "dwater/net/socket_ops.h"

#include <stdio.h> // for snprintf

using namespace dwater;
using namespace dwater::net;

namespace dwater {
namespace net {
namespace detail {

void RemoveConnection(EventLoop* loop, const TcpConnectionPtr& conn) {
    loop->QueueInLoop(std::bind(&TcpConnection::ConnectionDestroyed, conn));
}

void RemoveConnector(const ConnectorPtr& connector) {
    (void)connector;
}
} // namespace detail
} // namespace net
} // namespace dwater

TcpClient::TcpClient(EventLoop* loop,
                     const InetAddress& server_address,
                     const string& name)
    : loop_(CHECK_NOTNULL(loop)),
      connector_(new Connector(loop, server_address)),
      name_(name),
      connection_callback_(DefaultConnectionCallback),
      message_callback_(DefaultMessageCallback),
      retry_(false),
      connect_(true),
      next_conn_id_(1) {
    
          connector_->SetNewConnectionCallback(
                  std::bind(&TcpClient::NewConnection, this, _1)
                  );
          LOG_INFO << "TcpClient::TcpClient[" << name_ 
                   << "] - connector " << GetPointer(connector_);
}

TcpClient::~TcpClient() {
    LOG_INFO << "TcpClient::~TcpClient[" << name_
             << "] - connector " << GetPointer(connector_);
    TcpConnectionPtr conn;
    bool unique = false;
    {
        MutexLockGuard lock(mutex_);
        unique = connection_.unique();
        conn = connection_;
    }
    if ( conn ) {
        assert(loop_ == conn->GetLoop());
        CloseCallback cb = std::bind(&detail::RemoveConnection, loop_, _1);
        loop_->RunInLoop(
                std::bind(&TcpConnection::SetCloseCallback, conn, cb)
                );
        if ( unique ) {
            conn->ForceClose();
        }
    } else {
        connector_->Stop();
        loop_->RunAfter(1, std::bind(&detail::RemoveConnector, connector_));
    }
}

void TcpClient::Connect() {
    LOG_INFO << "TcpClient::Connect[" << name_ << "] - connecting to "
             << connector_->ServerAddress().ToIpPort();
    connect_ = true;
    connector_->Start();
}

void TcpClient::Disconnect() {
    connect_ = false;
    {
        MutexLockGuard lock(mutex_);
        if ( connection_ ) {
            connection_->Shutdown();
        }
    }
}

void TcpClient::Stop() {
    connect_ = false;
    connector_->Stop();
}

void TcpClient::NewConnection(int sockfd) {
    loop_->AssertInLoopThread();
    InetAddress peer_addr(socket::GetPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof(buf), ":%s#%d", peer_addr.ToIpPort().c_str(), next_conn_id_);
    ++next_conn_id_;
    string conn_name = name_ + buf;
    InetAddress local_addr(socket::GetLocalAddr(sockfd));
    TcpConnectionPtr conn(new TcpConnection(loop_,
                                            conn_name,
                                            sockfd,
                                            local_addr,
                                            peer_addr));
    conn->SetConnectionCallback(connection_callback_);
    conn->SetMessageCallback(message_callback_);
    conn->SetWriteCompleteCallback(write_complete_callback_);
    conn->SetCloseCallback(std::bind(&TcpClient::RemoveConnection, this, _1));
    {
        MutexLockGuard lock(mutex_);
        connection_ = conn;
    }
    conn->ConnectionEstablished();
}

void TcpClient::RemoveConnection(const TcpConnectionPtr& conn) {
    loop_->AssertInLoopThread();
    assert(loop_ == conn->GetLoop());

    {
        MutexLockGuard lock(mutex_);
        assert(connection_ == conn);
        connection_.reset();
    }

    loop_->QueueInLoop(std::bind(&TcpConnection::ConnectionDestroyed, conn));
    if ( retry_ && connect_ ) {
        LOG_INFO << "TcpClient::connect[" << name_ << "] - Reconnecting to "
                 << connector_->ServerAddress().ToIpPort();
        connector_->Restart();
    }
}
