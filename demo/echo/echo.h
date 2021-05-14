// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        
// Descripton:      A echo server build with dwater

#ifndef DWATER_DEMO_ECHO_ECHO_H
#define DWATER_DEMO_ECHO_ECHO_H

#include "dwater/net/tcp_server.h"

class EchoServer {
public:
    EchoServer(dwater::net::EventLoop* loop,
               const dwater::net::InetAddress& listen_addr);

    void Start();

private:
    void OnConnection(const dwater::net::TcpConnectionPtr& conn);

    void OnMessage(const dwater::net::TcpConnectionPtr& conn,
                   dwater::net::Buffer* buf,
                   dwater::Timestamp time);

    dwater::net::TcpServer server_;
};



#endif //  DWATER_DEMO_ECHO_ECHO_H






