// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.25
// Filename:        tcp_connection.h
// Descripton:       

#ifndef DWATER_NET_TCP_CONNECTION_H
#define DWATER_NET_TCP_CONNECTION_H

#include "dwater/base/noncopable.h"
#include "dwater/base/string_piece.h"
#include "dwater/base/types.h"
#include "dwater/net/callbacks.h"
#include "dwater/net/buffer.h"
#include "dwater/net/inet_address.h"

#include <memory>
#include <boost/any.hpp>

struct tcp_info; // in <netinet/tcp.h>

namespace dwater {

namespace net {

class Channel;
class EventLoop;
class Socket;

/// 
/// TcpConnection类，表示一个TCP连接，客服端、服务端都可以使用
///
/// 这是一个接口类，不需要暴露很多的细节
class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop* loop,
                  const string& name,
                  int sockfd,
                  const InetAddress& local_addr,
                  const InetAddress& peer_addr);
    ~TcpConnection();

    EventLoop* GetLoop() const { return loop_; }

    const string& Name() const {  return name_; };

    const InetAddress& LocalAddress() const { return local_addr_; }

    const InetAddress& PeerAddress() const { return peer_addr_; }

    bool Connected() const { return state_ == kconnected; }

    bool DisConnected() const { return state_ == kdisconnected; }

    bool GetTcpInfo(struct tcp_info*) const;

    string GetTcpInfoString() const;

    void Send(const void* message, int len);
    
    void Send(const StringPiece& message);

    void Send(Buffer* message);

    void Shutdown(); 

    void ForceClose();

    void ForceCloseWithDelay(double seconds);

    void SetTcpNoDelay(bool on);

    void StartRead();

    void StopRead();

    bool IsReading() const { return reading_; }

    void SetContext(const boost::any& context) { context_ = context; }

    const boost::any& GetContext() const { return context_; }

    boost::any* GetMutableContext() { return &context_; }

    void SetConnectionCallback(const ConnectionCallback& cb) {
        connection_callback_ = cb;
    }

    void SetMessageCallback(const MessageCallback& cb) {
        message_callback_ = cb;
    }

    void SetWriteCompleteCallback(const WriteCompleteCallback& cb) {
        write_complete_callback_ = cb;
    }

    void SetHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t high_water_mark) {
        high_water_mark_callback_ = cb;
        high_water_mark_ = high_water_mark;
    }

    void SetCloseCallback(const CloseCallback& cb) {
        close_callback_ = cb;
    }

    Buffer* InputBuffer() {
        return &input_buffer_;
    }

    Buffer* OutputBuffer() {
        return &output_buffer_;
    }

    void ConnectionEstablished();

    void ConnectionDestroyed();
private:
    enum StateE { kdisconnected, kconnecting, kconnected, kdisconnecting };
    void HandleRead(Timestamp receive_time);
    void HandleWrite();
    void HandleClose();
    void HandleError();

    void SendInLoop(const StringPiece& message);
    void SendInLoop(const void* Message, size_t len);
    void ShutdownInLoop();

    void ForceCloseInLoop();
    void SetState(StateE state) { state_ = state; }

    const char* StateToString() const;
    void StartReadInLoop();
    void StopReadInLoop();

    EventLoop*      loop_;
    const string    name_;
    StateE          state_;
    bool            reading_;

    std::unique_ptr<Socket>     socket_;
    std::unique_ptr<Channel>    channel_;
    const InetAddress           local_addr_;
    const InetAddress           peer_addr_;
    ConnectionCallback          connection_callback_;
    MessageCallback             message_callback_;
    WriteCompleteCallback       write_complete_callback_;
    HighWaterMarkCallback       high_water_mark_callback_;
    CloseCallback               close_callback_;
    size_t                      high_water_mark_;
    Buffer                      input_buffer_;
    Buffer                      output_buffer_;
    boost::any                  context_;
};
} // namespace net
} // namespace dwater
#endif // DWATER_NET_TCP_CONNECTION_H
