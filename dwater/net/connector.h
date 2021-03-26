// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.26
// Filename:        conector.h
// Descripton:      

#ifndef DWATER_NET_CONNECTOR_H
#define DWATER_NET_CONNECTOR_H

#include "dwater/base/noncopable.h"
#include "dwater/net/inet_address.h"

#include <functional>
#include <memory>

namespace dwater {

namespace net {

class Channel;
class EventLoop;

///
/// Connector给客户端使用，负责主动发起连接，具有重新连接和停止连接的功能
/// 
class Connector : noncopyable, public std::enable_shared_from_this<Connector> {
public:
    typedef std::function<void (int sockfd)> NewConnectionCallback;

    Connector(EventLoop* loop, const InetAddress& server_addr);

    ~Connector();

    void SetNewConnectionCallback(const NewConnectionCallback& cb) {
        new_connection_callback_ = cb;
    }

    const InetAddress& ServerAddress() const { return server_addr_; }

    void Start();

    void Restart();

    void Stop();

private:
    enum States { kdisconnected, kconnecting, kconnected };
    static const int kmax_retry_delay_ms = 30 * 1000;
    static const int kinit_retry_delay_ms = 500;

    void SetState(States s) {  state_ = s; }

    void StartInLoop();

    void StopInLoop();

    void Connect();

    void Connecting(int sockfd);

    void HandleWrite();

    void HandleError();

    void Retry(int sockfd);
    
    int RemoveAndResetChannel();

    void ResetChannel();
private:
    EventLoop*                  loop_;
    InetAddress                 server_addr_;
    bool                        connect_;
    States                      state_;
    std::unique_ptr<Channel>    channel_;
    NewConnectionCallback       new_connection_callback_;
    int                         retry_delay_ms_;
};
} // namespace net
} // namespace dwater
#endif // DWATER_NET_CONNECTOR_H
