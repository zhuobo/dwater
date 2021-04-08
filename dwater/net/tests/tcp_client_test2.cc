// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.04.08
// Filename:        tcp_client_test2.cc
// Descripton:       

#include "dwater/base/logging.h"
#include "dwater/base/thread.h"
#include "dwater/net/event_loop.h"
#include "dwater/net/tcp_client.h"

using namespace dwater;
using namespace dwater::net;

void threadFunc(EventLoop* loop) {
    InetAddress server_addr("127.0.0.1", 1234);
    TcpClient client(loop, server_addr, "TcpClient");
    client.Connect();
    current_thread::SleepUsec(1000 * 1000);
}

int main(int argc, char* argv[]) {
    Logger::SetLogLevel(Logger::DEBUG);

    EventLoop loop;
    loop.RunAfter(4.0, std::bind(&EventLoop::Quit, &loop));
    Thread thread(std::bind(threadFunc, &loop));
    thread.Start();
    loop.Loop();
}
