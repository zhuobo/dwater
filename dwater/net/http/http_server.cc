// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.04.09
// Filename:        http_server.cc
// Descripton:       

#include "dwater/net/http/http_server.h"

#include "dwater/base/logging.h"
#include "dwater/net/http/http_context.h"
#include "dwater/net/http/http_request.h"
#include "dwater/net/http/http_response.h"

using namespace dwater;
using namespace dwater::net;

namespace dwater {
namespace net {
namespace detail {
    void DefaultHttpCallback(const HttpRequest&, HttpResponse* resp) {
        resp->SetStatusCode(HttpResponse::k404NotFound);
        resp->SetStatusMessage("NOT FOUND");
        resp->SetCloseConnection(true);
    }
}
}
}

HttpServer::HttpServer(EventLoop* loop,
                       const InetAddress& listen_addr,
                       const string& name,
                       TcpServer::Option option)
    : server_(loop, listen_addr, name, option),
      http_callback_(detail::DefaultHttpCallback) {
    server_.SetConnectionCallback(std::bind(&HttpServer::OnConnection, this, _1));
    server_.SetMessageCallback(std::bind(&HttpServer::OnMessage, this, _1, _2, _3));
}

void HttpServer::Start() {
    LOG_WARN << "HttpServer[" << server_.Name()
             << "] starts listening on " << server_.IpPort();
    server_.Start();
}

void HttpServer::OnConnection(const TcpConnectionPtr& conn) {
    if ( conn->Connected() ) {
        conn->SetContext(HttpContext());
    }
}

void HttpServer::OnMessage(const TcpConnectionPtr& conn,
                           Buffer* buf,
                           Timestamp receive_time) {
    HttpContext* context = boost::any_cast<HttpContext>(conn->GetMutableContext());
    if ( !context->ParseRequest(buf, receive_time) ) {
        conn->Send("HTTP/1.1 400 Bad Requeset\r\n\r\n");
        conn->Shutdown();
    }

    if ( context->GotAll() ) {
        OnRequest(conn, context->Requeset());
        context->Reset();
    }
}

void HttpServer::OnRequest(const TcpConnectionPtr& conn, const HttpRequest& req) {
    const string& connection = req.GetHeader("Connection");
    bool close = connection == "close" ||
        (req.GetVersion() == HttpRequest::khttp10 && connection != "Keep-Alive");
    HttpResponse response(close);
    http_callback_(req, &response);
    Buffer buf;
    response.AppendToBuffer(&buf);
    conn->Send(&buf);
    if ( response.CloseConnection() ) {
        conn->Shutdown();
    }
}
