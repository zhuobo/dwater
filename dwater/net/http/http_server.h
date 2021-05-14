// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.04.09
// Filename:        http_server.h
// Descripton:       

#ifndef DWATER_NET_HTTP_HTTP_SERVER_H
#define DWATER_NET_HTTP_HTTP_SERVER_H

#include "dwater/net/tcp_server.h"

namespace dwater {
namespace net {

class HttpRequest;
class HttpResponse;

class HttpServer : noncopyable {
public:
    typedef std::function<void (const HttpRequest&, HttpResponse*)> HttpCallback;

    HttpServer(EventLoop* loop,
               const InetAddress& listen_addr,
               const string& name,
               TcpServer::Option = TcpServer::kno_reuse_port);

    EventLoop* GetLoop() const {
        return server_.GetLoop();
    }

    void SetHttpCallback(const HttpCallback& cb) {
        http_callback_ = cb;
    }

    void SetThreadNum(int num_threads) {
        server_.SetThreadNum(num_threads);
    }

    void Start();

private:
    void OnConnection(const TcpConnectionPtr& conn);

    void OnMessage(const TcpConnectionPtr& conn,
                   Buffer* buf,
                   Timestamp receive_time);

    void OnRequest(const TcpConnectionPtr&, const HttpRequest&);

    TcpServer       server_;
    HttpCallback    http_callback_;
};

} // dwater
} // net

#endif // DWATER_NET_HTTP_HTTP_SERVER_H
