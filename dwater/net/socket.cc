// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.24
// Filename:        socket.cc
// Descripton:       

#include "dwater/net/socket.h"

#include "dwater/base/logging.h"
#include "dwater/net/inet_address.h"
#include "dwater/net/socket_ops.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h> // for snprintf

using namespace dwater;
using namespace dwater::net;

Socket::~Socket() {
    socket::Close(sockfd_);
}

bool Socket::GetTcpInfo(struct tcp_info* tcpinfo) const {
    socklen_t len = sizeof(*tcpinfo);
    MemZero(tcpinfo, len);
    return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpinfo, &len) == 0;
}

bool Socket::GetTcpInfoString(char* buf, int len) const {
    struct tcp_info tcpinfo;
    bool done  = GetTcpInfo(&tcpinfo);
    if ( done ) {
        snprintf(buf, len, "unrecovered=%u "
             "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
             "lost=%u retrans=%u rtt=%u rttvar=%u "
             "sshthresh=%u cwnd=%u total_retrans=%u",
             tcpinfo.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
             tcpinfo.tcpi_rto,          // Retransmit timeout in usec
             tcpinfo.tcpi_ato,          // Predicted tick of soft clock in usec
             tcpinfo.tcpi_snd_mss,
             tcpinfo.tcpi_rcv_mss,
             tcpinfo.tcpi_lost,         // Lost packets
             tcpinfo.tcpi_retrans,      // Retransmitted packets out
             tcpinfo.tcpi_rtt,          // Smoothed round trip time in usec
             tcpinfo.tcpi_rttvar,       // Medium deviation
             tcpinfo.tcpi_snd_ssthresh,
             tcpinfo.tcpi_snd_cwnd,
             tcpinfo.tcpi_total_retrans);  // Total retransmits for entire connection
    }
    return done;
}

void Socket::BindAddress(const InetAddress& addr) {
    socket::BindOrDie(sockfd_, addr.GetSockAddr());
}

void Socket::Listen() {
    socket::ListenOrDie(sockfd_);
}

int Socket::Accept(InetAddress* peer_addr) {
    struct sockaddr_in6 addr;
    MemZero(&addr, sizeof(addr));
    int connfd = socket::Accept(sockfd_, &addr);
    if ( connfd >= 0 ) {
        peer_addr->SetSockAddrInet6(addr);
    }
    return connfd;
}

void Socket::ShutdownWrite() {
    socket::ShutdownWrite(sockfd_);
}

void Socket::SetTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof(optval)));
}

void Socket::SetReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof(optval)));
}

void Socket::SetReusePort(bool on) {
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof(optval)));
    if ( ret < 0 && on ) {
        LOG_SYSERR << "SO_REUSEPORT failed.";
    }
#else
    if ( on ) {
        LOG_ERROR << "SO_REUSEPORT is not supported.";
    }
#endif
}

void Socket::SetKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof(optval)));
}
