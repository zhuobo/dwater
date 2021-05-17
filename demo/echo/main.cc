// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        
// Descripton:       


#include "demo/echo/echo.h"

#include "dwater/base/logging.h"
#include "dwater/net/event_loop.h"

#include <unistd.h>


int main() {
    LOG_INFO << " PID = " << getpid();
    dwater::net::EventLoop loop;
    dwater::net::InetAddress listen_addr(2008);
    EchoServer server(&loop, listen_addr);
    server.Start();
    loop.Loop();
}

