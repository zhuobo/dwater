// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.24
// Filename:        socket.h
// Descripton:       

#ifndef DWATER_NET_SOCKET_H
#define DWATER_NET_SOCKET_H
#include "dwater/base/noncopable.h"

struct tcp_info; // <netinet/tcp.h>

namespace dwater {
namespace net {

class InetAddress;

///
/// socket的包装类
///
/// 当Socket被析构的时候会关闭sockfd
/// Socket是线程安全的
/// 
class Socket : noncopyable {
public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}

    ~Socket();

    ///
    /// 返回文件描述符
    /// 
    int Fd() const { return sockfd_; }

    ///
    /// sockfd和IP，port绑定
    /// 
    /// 如果参数local_addr已经在使用，进程终止
    /// 
    void BindAddress(const InetAddress& local_addr);

    ///
    /// 监听端口
    /// 
    /// 如果参数local_addr已经在使用，进程终止
    /// 
    void Listen();

    ///
    /// 成功是返回true
    /// 
    bool GetTcpInfo(struct tcp_info*) const;

    ///
    /// 成功是返回true
    /// 
    bool GetTcpInfoString(char* buf, int len)  const;

    ///
    /// 接受连接
    /// 
    /// 如果成功连接，返回被接受socket的fd, 这个fd已经被设置为Non-block
    /// close-on-exe,如果失败返回-1
    /// 
    int Accept(InetAddress* peer_addr);

    ///
    /// 关掉写的一半链接
    /// 
    void ShutdownWrite();
 
    ///
    /// 设置TCP-NODELAY，也就是关闭Nagle算法
    /// 
    void SetTcpNoDelay(bool on);

    ///
    /// 开启关闭 SO_REUSEADDR
    /// 
    void SetReuseAddr(bool on);

    ///
    /// 开启关闭 SO_REUSEPORT
    /// 
    void SetReusePort(bool on);

    ///
    /// 开启关闭 SO_KEEPALIVE
    /// 
    void SetKeepAlive(bool on);
private:
    const int sockfd_;
}; // class Socket
} // namespace net
} // namespace dwter

#endif // DWATER_NET_SOCKET_H
