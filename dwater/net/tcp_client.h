// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.04.08
// Filename:        tcp_client.h
// Descripton:       

#ifndef DWATER_NET_TCP_CLIENT_H
#define DWATER_NET_TCP_CLIENT_H

#include "dwater/base/mutex.h"
#include "dwater/net/tcp_connection.h"

namespace dwater {

namespace net {

class Connector;

typedef std::shared_ptr<Connector> ConnectorPtr;

class TcpClient : noncopyable {
public:
    TcpClient(EventLoop* loop, const InetAddress& server_addr, const string& name);

    ~TcpClient();

    void Connect();

    void Disconnect();

    void Stop();

    TcpConnectionPtr Connection() const {
        MutexLockGuard lock(mutex_);
        return connection_;
    }

    EventLoop* GetLoop() const {
        return loop_;
    }

    bool Retry() const {
        return retry_;
    }

    void EnableRetry() {
        retry_ = true;
    }

    const string& Name() const {
        return name_;
    }

    void SetConnectionCallback(ConnectionCallback cb) {
        connection_callback_ =std::move(cb);
    }

    void SetMessageCallback(MessageCallback cb) {
        message_callback_ =std::move(cb);
    }

    void SetWriteCompleteCallback(WriteCompleteCallback cb) {
        write_complete_callback_ =std::move(cb);
    }

private:
    void NewConnection(int sockfd);

    void RemoveConnection(const TcpConnectionPtr& conn);

    EventLoop*                          loop_;
    ConnectorPtr                        connector_;
    const string                        name_;
    ConnectionCallback                  connection_callback_;
    MessageCallback                     message_callback_;
    WriteCompleteCallback               write_complete_callback_;
    bool                                retry_;
    bool                                connect_;
    int                                 next_conn_id_;
    mutable MutexLock                   mutex_;
    TcpConnectionPtr                    connection_ GUARDED_BY(mutex_);
};

} // namespace net
} // namespace dwater

#endif // DWATER_NET_TCP_CLIENT_H
