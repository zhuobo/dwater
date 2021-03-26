// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        
// Descripton:       

#include "dwater/net/tcp_server.h"
#include "dwater/net/event_loop.h"
#include "dwater/net/inet_address.h"
#include <stdio.h>

using namespace dwater;
using namespace dwater::net;

void onConnection(const TcpConnectionPtr& conn) {
    if (  conn->Connected() ) {
        printf("connected\n");
    } else {
        printf("noconnected\n");
    }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receivetime) {
    printf("message");
}

int main() {
    printf("mian\n");

    InetAddress listen_addr(9981);
    EventLoop loop;

    TcpServer server(&loop, listen_addr, "testing-server");
    server.SetConnectionCallback(onConnection);
    server.SetMessageCallback(onMessage);
    server.Start();
    loop.Loop();
}


