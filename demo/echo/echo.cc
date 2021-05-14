// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        
// Descripton:       


#include "demo/echo/echo.h"
#include "dwater/base/logging.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

EchoServer::EchoServer(dwater::net::EventLoop* loop,
                       const dwater::net::InetAddress& listen_addr) 
    : server_(loop, listen_addr, "EchoServer") {
    server_.SetConnectionCallback(std::bind(&EchoServer::OnConnection, this, _1));
    server_.SetMessageCallback(std::bind(&EchoServer::OnMessage, this, _1, _2, _3));
}

void EchoServer::Start() {
    server_.Start();
}

void EchoServer::OnConnection(const dwater::net::TcpConnectionPtr& conn) {
    LOG_INFO << "EchoServer - " << conn->PeerAddress().ToIpPort() << " -> "
             << conn->LocalAddress().ToIpPort() << " is "
             << (conn->Connected() ? "UP" : "DOWN");
}

void EchoServer::OnMessage(const dwater::net::TcpConnectionPtr& conn,
                           dwater::net::Buffer* buf,
                           dwater::Timestamp time) {
    dwater::string msg(buf->RetrieveAllAsString());
    LOG_INFO << conn->Name() << " echo " << msg.size() << " bytes, "
             << " data received at " << time.ToString();
    conn->Send(msg);
}
