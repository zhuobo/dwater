// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.25
// Filename:        acceptor.h
// Descripton:       

#ifndef DWATER_NET_ACCEPTOR_H
#define DWATER_NET_ACCEPTOR_H

#include "dwater/net/socket.h"
#include "dwater/net/channel.h"

#include <functional>

namespace dwater {

namespace net {

/// 
/// Acceptor 用于accept新的TCP连接，通过回调函数通知使用者，是TCPServer的内部类
/// 
class Acceptor : noncopyable {
public:
    typedef std::function<void(int sockfd, const InetAddress&)> NewConnectionCallback;

    Acceptor(EventLoop* loop, const InetAddress& listen_addr, bool reuse_port);
    ~Acceptor();

    void SetNewConnnectionCallback(const NewConnectionCallback& cb) {
        new_connection_callback_ = cb;
    }

    void Listen();

    bool Listening() const {
        return listening_;
    }

private:
    void HandleRead();

    EventLoop*                  loop_;
    Socket                      accept_socket_;
    Channel                     accept_channel_;
    NewConnectionCallback       new_connection_callback_;
    bool                        listening_;
    int                         idle_fd_;
};// class Acceptor

} // namespace net

} // namespace dwater

#endif // DWATER_NET_ACCEPTOR_H



