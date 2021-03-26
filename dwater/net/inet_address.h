// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.24
// Filename:        inet_address.h
// Descripton:       

#ifndef DWATER_NET_INET_ADDRESS_H
#define DWATER_NET_INET_ADDRESS_H

#include "dwater/base/copyable.h"
#include "dwater/base/string_piece.h"

#include <netinet/in.h>

namespace dwater {

namespace net {

namespace socket {

const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);

} // namespace socket

///
/// sockaddr_in的包装类，封装了sockaddr_in的常用操作
///
/// 这是一个POD接口类
/// 
class InetAddress : public dwater::copyable {
public:
    ///
    /// 根据给定的端口号初始化一个地址，通常会被TCPServer监听
    ///
    explicit InetAddress(uint16_t port = 0, bool loopback_only = false, bool ipv6 = false);

    /// 
    /// 根据给定的IP、端口号初始化一个地址，IP地址格式需要正确
    ///
    InetAddress(StringArg ip, uint16_t port, bool ipv6 = false);

    /// 
    /// 根据一个sockaddr_in构造
    ///
    explicit InetAddress(const struct sockaddr_in& addr) : addr_(addr) {}

    /// 
    /// 根据一个个sockaddr_in6构造
    ///
    explicit InetAddress(const struct sockaddr_in6& addr) : addr6_(addr) {}

    ///
    /// 返回地址族信息sin_family
    /// 
    sa_family_t Family() const {
        return addr_.sin_family;
    }

    string ToIp() const;
    string ToIpPort() const;
    uint16_t Port() const;

    const struct sockaddr* GetSockAddr() const {
        return socket::sockaddr_cast(&addr6_);
    }

    void SetSockAddrInet6(const struct sockaddr_in6& addr6) { addr6_ = addr6; }

    uint32_t Ipv4NetEndian() const;
    uint16_t PortNetEndian() const { return addr_.sin_port; }

    ///
    /// 把给定字符串解析成IP地址的形式
    /// 
    static bool Resolve(StringArg hostname, InetAddress* result);

    /// 
    /// 设置scope id
    ///
    void SetScopeId(uint32_t scope_id);
private:
    union {
        struct sockaddr_in  addr_;
        struct sockaddr_in6 addr6_;
    };
}; // class InetAddress

} // namespace net
} // namespace dwater

#endif // DWATER_NET_INET_ADDRESS_H

