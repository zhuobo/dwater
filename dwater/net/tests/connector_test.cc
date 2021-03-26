// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.26
// Filename:        connector_test.cc
// Descripton:       

#include "dwater/net/connector.h"
#include "dwater/net/event_loop.h"
#include "dwater/net/inet_address.h"

#include <stdio.h>

using namespace dwater;
using namespace dwater::net;

EventLoop* g_loop;

void connect_callback(int sockfd) {
    printf("connected\n");
    g_loop->Quit();
}

int main() {
    EventLoop loop;
    g_loop = &loop;
    InetAddress addr("127.0.0.1", 9989);
    Connector* connector = new Connector(&loop, addr);
    connector->SetNewConnectionCallback(connect_callback);
    connector->Start();
    loop.Loop();
}

