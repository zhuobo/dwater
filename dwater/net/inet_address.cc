// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.25
// Filename:        inet_address.cc
// Descripton:       

#include "dwater/net/inet_address.h"

#include "dwater/net/endian.h"
#include "dwater/net/socket_ops.h"
#include "dwater/base/logging.h"

#include <netdb.h>
#include <netinet/in.h>


#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kinaddr_any = INADDR_ANY;
static const in_addr_t kinaddr_loopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"

// 两个描述socke地址的结构体
/*******************************
struct sockaddr_in {
    sa_family_t     sin_family;
    uint16_t        sin_port;
    struct in_addr  sin_addr;
};

struct in_addr {
    uint32_t        s_addr;
};

struct sockaddr_in6 {
    sa_family_t     sin6_family;
    uint16_t        sin6_port;
    uint32_t        sin6_flowinfo;
    struct in6_addr sin6_addr;
    uint32_t        sin6_scope_id;
};
**********************************/

using namespace dwater;
using namespace dwater::net;

static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6),
    "InetAddress is same size of as sockaddr_in6");
static_assert(offsetof(sockaddr_in, sin_family) == 0,"sin_family offset 0");
static_assert(offsetof(sockaddr_in6, sin6_family) == 0,"sin6_family offset 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset 2");
static_assert(offsetof(sockaddr_in6, sin6_port) == 2, "sin6_port offset 2");


InetAddress::InetAddress(uint16_t port, bool loopback_only, bool ipv6) {
    static_assert(offsetof(InetAddress, addr6_) == 0, "addr6_  offset 0");
    static_assert(offsetof(InetAddress, addr_) == 0, "addr_ offset 0");
    if ( ipv6 ) {
        MemZero(&addr6_, sizeof(addr6_));
        addr6_.sin6_family = AF_INET6;
        in6_addr ip = loopback_only ? in6addr_loopback : in6addr_any;
        addr6_.sin6_addr = ip;
        addr6_.sin6_port = socket::HostToNetwork16(port);
    } else {
        MemZero(&addr_, sizeof(addr_));
        addr_.sin_family = AF_INET;
        in_addr_t ip = loopback_only ?  kinaddr_loopback : kinaddr_any;
        addr_.sin_addr.s_addr = socket::HostToNetwork32(ip);
        addr_.sin_port = socket::HostToNetwork16(port);
    }
}

InetAddress::InetAddress(StringArg ip, uint16_t port, bool ipv6) {
    if ( ipv6 || strchr(ip.Cstr(), ':') ) {
        MemZero(&addr6_, sizeof(addr6_));
        socket::FromIpPort(ip.Cstr(), port, &addr6_);
    } else {
        MemZero(&addr_, sizeof(addr_));
        socket::FromIpPort(ip.Cstr(), port, &addr_);
    }
}

string InetAddress::ToIpPort() const {
    char buf[64] = "";
    socket::ToIpPort(buf, sizeof(buf), GetSockAddr());
    return  buf;
}

string InetAddress::ToIp() const {
    char buf[64] = "";
    socket::ToIp(buf, sizeof(buf), GetSockAddr());
    return buf;
}

uint32_t InetAddress::Ipv4NetEndian() const {
    assert(Family() == AF_INET);
    return addr_.sin_addr.s_addr;
}

uint16_t InetAddress::Port() const {
    return socket::HostToNetwork16(PortNetEndian());
}

static __thread char t_resolve_buffer[64 * 1024];

bool InetAddress::Resolve(StringArg hostname, InetAddress* out) {
    assert(out != NULL);
    struct hostent hent;
    struct hostent* he = NULL;
    int herrno = 0;
    MemZero(&hent, sizeof(hent));

    int ret = gethostbyname_r(hostname.Cstr(), &hent, t_resolve_buffer,
                            sizeof(t_resolve_buffer), &he, &herrno);
    if ( ret == 0 && he != NULL ) {
        assert(he->h_addrtype == AF_INET &&  he->h_length == sizeof(uint32_t));
        out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
        return true;
    } else {
        if ( ret ) {
            LOG_SYSERR << "InetAddress::Resolve";
        }
        return false;
    }
}


void InetAddress::SetScopeId(uint32_t scope_id) {
    if ( Family() == AF_INET6 ) {
        addr6_.sin6_scope_id = scope_id;
    }
}
