// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.26
// Filename:        tcp_server.cc
// Descripton:       

#include "dwater/net/tcp_server.h"

#include "dwater/base/logging.h"
#include "dwater/net/acceptor.h"
#include "dwater/net/event_loop.h"
#include "dwater/net/event_loop_thread_pool.h"
#include "dwater/net/socket_ops.h"

#include <stdio.h>

using namespace dwater;
using namespace dwater::net;

TcpServer::TcpServer(EventLoop* loop,
                     const InetAddress& listen_addr,
                     const string& name,
                     Option option) 
    : loop_(CHECK_NOTNULL(loop)),
      ip_port_(listen_addr.ToIpPort()),
      name_(name),
      acceptor_(new Acceptor(loop, listen_addr, option == kreuser_port)),
      thread_pool_(new EventLoopThreadPool(loop, name_)),
      connection_callback_(DefaultConnectionCallback),
      message_callback_(DefaultMessageCallback),
      next_connid_(1) {
          acceptor_->SetNewConnnectionCallback(
                  std::bind(&TcpServer::NewConnection, this, _1, _2)
                  );
}

TcpServer::~TcpServer() {
    loop_->AssertInLoopThread();
    LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";
    for ( auto& item : connections_ ) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->GetLoop()->RunInLoop(
                std::bind(&TcpConnection::ConnectionDestroyed, conn)
                );
    }
}

void TcpServer::SetThreadNum(int num_thread) {
    assert(num_thread >= 0);
    thread_pool_->SetThreadNum(num_thread);
}

void TcpServer::Start() {
    if ( started_.GetAndSet(1) == 0 ) {
        thread_pool_->Start(thread_init_callback_);

        assert(!acceptor_->Listening());
        loop_->RunInLoop(std::bind(&Acceptor::Listen, GetPointer(acceptor_)));
    }
}

void TcpServer::NewConnection(int sockfd, const InetAddress& peer_addr) {
    loop_->AssertInLoopThread();
    EventLoop* io_loop = thread_pool_->GetNextLoop();
    char buf[64];
    snprintf(buf, sizeof(buf), "-%s#%d", ip_port_.c_str(), next_connid_);
    ++next_connid_;
    string conn_name = name_ + buf;
    LOG_INFO << "TcpServer::NewConnection [" << name_
             << "] - new connection [" << conn_name
             << "] from " << peer_addr.ToIpPort();
    InetAddress local_addr(socket::GetLocalAddr(sockfd));
    TcpConnectionPtr conn(new TcpConnection(io_loop, conn_name, sockfd, local_addr, peer_addr));
    connections_[conn_name] = conn;
    conn->SetConnectionCallback(connection_callback_);
    conn->SetMessageCallback(message_callback_);
    conn->SetWriteCompleteCallback(write_complete_callback_);
    conn->SetCloseCallback(std::bind(&TcpServer::RemoveConnection, this, _1));
    io_loop->RunInLoop(std::bind(&TcpConnection::ConnectionEstablished, conn));
}

void TcpServer::RemoveConnection(const TcpConnectionPtr& conn) {
    loop_->RunInLoop(std::bind(&TcpServer::RemoveConenctionInLoop, this, conn));
}

void TcpServer::RemoveConenctionInLoop(const TcpConnectionPtr& conn) {
    loop_->AssertInLoopThread();
    LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
             << "] - connection " << conn->Name();
    size_t n = connections_.erase(conn->Name());
    (void)n;
    assert(n == 1);
    EventLoop* io_loop = conn->GetLoop();
    io_loop->QueueInLoop(std::bind(&TcpConnection::ConnectionDestroyed, conn));
}
