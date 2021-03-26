// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.25
// Filename:        acceptor.cc
// Descripton:       

#include "dwater/net/acceptor.h"
#include "dwater/base/logging.h"
#include "dwater/net/event_loop.h"
#include "dwater/net/inet_address.h"
#include "dwater/net/socket_ops.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

using namespace dwater;
using namespace dwater::net;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listen_addr, bool reuse_port)
    : loop_(loop),
    accept_socket_(socket::CreateNonblockingOrDie(listen_addr.Family())),
    accept_channel_(loop, accept_socket_.Fd()),
    listening_(false),
    idle_fd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
    assert(idle_fd_ >= 0);
    accept_socket_.SetReuseAddr(true);
    accept_socket_.SetReusePort(reuse_port);
    accept_socket_.BindAddress(listen_addr),
        accept_channel_.SetReadCallback(std::bind(&Acceptor::HandleRead, this));
}

Acceptor::~Acceptor() {
    accept_channel_.DisableAll();
    accept_channel_.Remove();
    ::close(idle_fd_);
}

void Acceptor::Listen() {
    loop_->AssertInLoopThread();
    listening_ = true;
    accept_socket_.Listen();
    accept_channel_.EnableReading();
}

void Acceptor::HandleRead() {
    loop_->AssertInLoopThread();
    InetAddress peer_addr;
    int connfd = accept_socket_.Accept(&peer_addr);
    if ( connfd >= 0 ) {
        if ( new_connection_callback_ ) {
            new_connection_callback_(connfd, peer_addr);
        } else {
            socket::Close(connfd);
        }
    } else {
        LOG_SYSERR << "in Acceptor::HandleRead";
        if ( errno == EMFILE ) {
            ::close(idle_fd_);
            idle_fd_ = ::accept(accept_socket_.Fd(), NULL, NULL);
            ::close(idle_fd_);
            idle_fd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}
