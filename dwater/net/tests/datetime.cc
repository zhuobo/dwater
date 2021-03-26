// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.25
// Filename:        acceptor.cc
// Descripton:       

#include "dwater/net/inet_address.h"
#include  "dwater/net/acceptor.h"
#include "dwater/net/socket.h"
#include "dwater/net/socket_ops.h"
#include "dwater/net/event_loop.h"
#include "dwater/base/timestamp.h"

#include "dwater/base/date.h"


#include <stdio.h>

void NewConnection(int sockfd, const dwater::net::InetAddress& peer_addr) {
    dwater::Timestamp now = dwater::Timestamp::Now();
    dwater::string str = now.ToFormattedString(false);
    ::write(sockfd, str.c_str(), strlen(str.c_str()));
    dwater::net::socket::Close(sockfd);
}

int main() {
    printf("main(): pid = %d\n", getpid());
    dwater::net::InetAddress listen_addr(9981);
    dwater::net::EventLoop loop;

    dwater::net::Acceptor acceptor(&loop, listen_addr,  false);
    acceptor.SetNewConnnectionCallback(NewConnection);
    acceptor.Listen();
    loop.Loop();
}


