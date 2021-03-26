// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.22
// Filename:        socket_ops.cc
// Descripton:      封装了socket的操作

#include "dwater/net/socket_ops.h"

#include "dwater/base/logging.h"
#include "dwater/base/types.h"
#include "dwater/net/endian.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/uio.h> // for struct iovec
#include <unistd.h>

using namespace dwater;
using namespace dwater::net;

namespace {

typedef struct sockaddr SA;

#if VALGRIND || defined (NO_ACCEPT4)
void SetNonblockAndCloseOnExec(int sockfd) {
    int old_option = ::fcntl(sockfd, F_GETFL, 0);
    int new_option = old_option | O_NONBLOCK;
    int ret = ::fcntl(sockfd, F_SETFL, new_option);

    // close on exec
    new_option = ::fcntl(sockfd, F_GETFD, 0);
    new_option |= FD_CLOEXEC;
    ret = ::fcntl(sockfd, F_SETFD, new_option);
    (void)ret;
}
#endif
} // unname namespace 

const struct sockaddr* socket::sockaddr_cast(const struct sockaddr_in6* addr) {
    return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

struct sockaddr* socket::sockaddr_cast(struct sockaddr_in6* addr) {
  return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

const struct sockaddr* socket::sockaddr_cast(const struct sockaddr_in* addr) {
  return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

const struct sockaddr_in* socket::sockaddr_in_cast(const struct sockaddr* addr) {
  return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}

const struct sockaddr_in6* socket::sockaddr_in6_cast(const struct sockaddr* addr) {
  return static_cast<const struct sockaddr_in6*>(implicit_cast<const void*>(addr));
}


int socket::CreateNonblockingOrDie(sa_family_t family) {
#if VALGRIND 
    int sockfd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);
    if ( sockfd < 0 ) {
        LOG_SYSFATAL << "socket::CreateNonblockingOrDie";
    }
    SetNonblockAndCloseOnExec(sockfd);
#else
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if ( sockfd < 0 ) {
        LOG_SYSFATAL << "socket::CreateNonblockingOrDie";
    }
#endif
    return sockfd;
}

void socket::BindOrDie(int sockfd, const struct sockaddr* addr) {
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if ( ret < 0 ) {
        LOG_SYSFATAL << "socket::BindOrDie";
    }
}

void socket::ListenOrDie(int sockfd) {
    int ret = ::listen(sockfd, SOMAXCONN);
    if ( ret < 0 ) {
        LOG_SYSFATAL << "socket::ListenOrDie";
    }
}

int socket::Accept(int sockfd, struct sockaddr_in6* addr) {
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
#if VALGRIND || defined (NO_ACCEPT4)
    int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
    SetNonblockAndCloseOnExec(connfd);
#else
    int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
    if ( connfd < 0 ) {
        int saved_errno = errno;
        LOG_SYSERR << "socket:Accept";
        switch (saved_errno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO:
            case EMFILE:
                errno = saved_errno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                LOG_FATAL << "unexpected error of ::accept " << saved_errno;
                break;
            default:
                LOG_FATAL << "unknow error of ::accept " << saved_errno;
        }
    }
    return connfd;
}

int socket::Connect(int sockfd, const struct sockaddr* addr) {
    return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

ssize_t socket::Read(int sockfd, void* buf, size_t count) {
    return ::read(sockfd, buf, count);
}

ssize_t socket::Readv(int sockfd, const struct iovec* iov, int iovcnt) {
    return ::readv(sockfd, iov, iovcnt);
}

ssize_t socket::Write(int sockfd, const void* buf, size_t count) {
    return ::write(sockfd, buf, count);
}

void socket::Close(int sockfd) {
    if ( ::close(sockfd) < 0 ) {
        LOG_SYSERR << "socket::Close";
    }
}

void socket::ShutdownWrite(int sockfd) {
    if ( ::shutdown(sockfd, SHUT_WR) < 0 ) {
        LOG_SYSERR << "socket::ShutdownWrite";
    }
}

// 把addr中的ip、port写入到buf中
void socket::ToIpPort(char* buf, size_t size, const struct sockaddr* addr) {
    if ( addr->sa_family == AF_INET6 ) {
        buf[0] = '[';
        ToIp(buf + 1, size - 1, addr);
        size_t end = ::strlen(buf);
        const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
        uint16_t port = socket::HostToNetwork16(addr6->sin6_port);
        assert(size > end);
        snprintf(buf + end, size - end, "]:%u", port);
        return;
    }
    ToIp(buf, size, addr);
    size_t end = ::strlen(buf);
    const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
    uint16_t port = socket::HostToNetwork16(addr4->sin_port);
    assert(size > end);
    snprintf(buf + end, size - end, ":%u", port);
}

// write ip of addr into buf
void socket::ToIp(char* buf, size_t size, const struct sockaddr* addr) {
    if ( addr->sa_family == AF_INET ) {
        assert(size >= INET_ADDRSTRLEN);
        const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
        ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
    } else if ( addr->sa_family == AF_INET6 ) {
        assert(size >= INET_ADDRSTRLEN);
        const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
        ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
    }
}

void socket::FromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = HostToNetwork16(port);
    if ( ::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0 ) {
        LOG_SYSERR << "socket::FromIpPort";
    }
}

void socket::FromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr) {
    addr->sin6_family = AF_INET6;
    addr->sin6_port = HostToNetwork16(port);
    if ( ::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0 ) {
        LOG_SYSERR << "socket::FromIpPort";
    }
}

int socket::GetSocketError(int sockfd) {
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);

    if ( ::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0 ) {
        return errno;
    } else {
        return optval;
    }
}

struct sockaddr_in6 socket::GetLocalAddr(int sockfd) {
    struct sockaddr_in6 local_addr;
    MemZero(&local_addr, sizeof(local_addr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(local_addr));
    if ( ::getsockname(sockfd, sockaddr_cast(&local_addr), &addrlen) < 0 ) {
        LOG_SYSERR << "socket::GetLocalAddr";
    }
    return local_addr;
}

struct sockaddr_in6 socket::GetPeerAddr(int sockfd) {
    struct sockaddr_in6 peer_addr;
    MemZero(&peer_addr, sizeof(peer_addr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(peer_addr));
    if ( ::getpeername(sockfd, sockaddr_cast(&peer_addr), &addrlen) < 0 ) {
        LOG_SYSERR << "socket::GetPeerAddr";
    }
    return peer_addr;
}

// 是否自己根自己连接，判断IP以及端口
bool socket::IsSelfConnect(int sockfd) {
    struct sockaddr_in6 localaddr = GetLocalAddr(sockfd);
    struct sockaddr_in6 peeraddr = GetPeerAddr(sockfd);
    if (localaddr.sin6_family == AF_INET) {
        const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr);
        const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
        return laddr4->sin_port == raddr4->sin_port
            && laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
    } else if (localaddr.sin6_family == AF_INET6) {
        return localaddr.sin6_port == peeraddr.sin6_port
        && memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof localaddr.sin6_addr) == 0;
    } else {
        return false;
    }   
}
