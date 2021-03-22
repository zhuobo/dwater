// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.22
// Filename:        socket_ops.h
// Descripton:      Encapsulates the basic operations of sockets

#ifndef DWATER_NET_SOCKET_OPS_H
#define DWATER_NET_SOCKET_OPS_H 

#include <arpa/inet.h>

namespace dwater {

namespace net {

namespace socket {
// create a nonblocking socket or abort if any error
int CreateNonblockingOrDie(sa_family_t family);

int Connect(int sockfd, const struct sockaddr* addr);

void BindOrDie(int sockfd, const struct sockaddr* addr);

void ListenOrDie(int sockfd);

int Accept(int sockfd, struct sockaddr_in6* addr);

ssize_t Read(int sockfd, void* buf, size_t count);

ssize_t Readv(int sockfd, const iovec* iov, int iovcnt);

ssize_t Write(int sockfd, const void* buf, size_t count);

void Close(int sockfd);

void ShutdownWrite(int sockfd);

void ToIpPort(char* buf, size_t, const struct sockaddr* addr);

void ToIp(char* buf, size_t size, const struct sockaddr* addr);

void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);

void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr);

int GetSocketError(int sockfd);

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr);
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);
const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr);

struct sockaddr_in6 GetLocalAddr(int sockfd);
struct sockaddr_in6 GetPeerAddr(int sockfd);

bool IsSelfConnect(int sockfd);
}// namespace socket
} // namespace net;
} // namespace dwater

#endif // DWATER_NET_SOCKET_OPS_H 
