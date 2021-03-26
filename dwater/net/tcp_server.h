// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.26
// Filename:        tcp_server.h
// Descripton:       

#ifndef DWATER_NET_TCP_SERVER_H
#define DWATER_NET_TCP_SERVER_H

#include "dwater/base/atomic.h"
#include "dwater/net/tcp_connection.h"
#include "dwater/base/types.h"

#include <map>

namespace dwater {

namespace net {

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

/// 
/// TcpServer 支持多线程单线程
/// 
class TcpServer : noncopyable {
public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;
    enum Option {
        kno_reuse_port,
        kreuser_port,
    };

    TcpServer(EventLoop* loop,
            const InetAddress& listen_addr,
            const string& name,
            Option option = kno_reuse_port);

    ~TcpServer();

    const string& IpPort() const { return ip_port_; }

    const string& Name() const { return name_; }

    EventLoop* GetLoop() const { return loop_; }

    void SetThreadNum(int num_thread);

    void SetThreadInitCallback(const ThreadInitCallback& cb) {
        thread_init_callback_ = cb;
    }

    std::shared_ptr<EventLoopThreadPool> ThreadPool() {
        return thread_pool_;
    }

    void Start();

    void SetConnectionCallback(const ConnectionCallback& cb) {
        connection_callback_ = cb;
    }

    void SetMessageCallback(const MessageCallback& cb) {
        message_callback_ = cb;
    }

    void SetWriteCompleteCallback(const WriteCompleteCallback& cb) {
        write_complete_callback_ = cb;
    }
private:
    void NewConnection(int sockfd, const InetAddress& peer_addr);

    void RemoveConnection(const TcpConnectionPtr& conn);
    void RemoveConenctionInLoop(const TcpConnectionPtr& conn);

    typedef std::map<string, TcpConnectionPtr> ConnectionMap;

    EventLoop*                              loop_;
    const string                            ip_port_;
    const string                            name_;
    std::unique_ptr<Acceptor>               acceptor_;
    std::shared_ptr<EventLoopThreadPool>    thread_pool_;
    ConnectionCallback                      connection_callback_;
    MessageCallback                         message_callback_;
    WriteCompleteCallback                   write_complete_callback_;
    ThreadInitCallback                      thread_init_callback_;
    AtomicInt32                             started_;

    int                                     next_connid_;
    ConnectionMap                           connections_;
};

} // namespace 
} // namespace dwater
#endif // DWATER_NET_TCP_SERVER_H
