// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.04.09
// Filename:        tcp_client_test3.cc
// Descripton:       

#include "dwater/base/logging.h"
#include "dwater/net/event_loop.h"
#include "dwater/net/event_loop_thread.h"
#include "dwater/net/tcp_client.h"

using namespace dwater;
using namespace dwater::net;

int main() {
    Logger::SetLogLevel(Logger::DEBUG);


    EventLoopThread loop_thread;
    {
        InetAddress server_addr("127.0.0.1", 9988);
        TcpClient client(loop_thread.StartLoop(), server_addr, "TcpClient");
        client.Connect();
        current_thread::SleepUsec(500 * 1000);
        client.Disconnect();
    }

    current_thread::SleepUsec(1000 * 1000);
}
