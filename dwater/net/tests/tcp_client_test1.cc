// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.04.08
// Filename:        tcp_client_test1.cc
// Descripton:       

#include "dwater/base/logging.h"
#include "dwater/net/event_loop.h"
#include "dwater/net/tcp_client.h"

using namespace dwater;
using namespace dwater::net;

TcpClient* g_client;

void timeout() {
    LOG_INFO << "timeout";
    g_client->Stop();
}

int main(int argc, char* argv[]) {
    EventLoop loop;
    InetAddress server_addr("127.0.0.1", 2);
    TcpClient client(&loop, server_addr, "TcpClient");
    g_client = &client;
    loop.RunAfter(0.0, timeout);
    loop.RunAfter(1.0, std::bind(&EventLoop::Quit, &loop));
    client.Connect();
    current_thread::SleepUsec(100 * 1000);
    loop.Loop();
}
