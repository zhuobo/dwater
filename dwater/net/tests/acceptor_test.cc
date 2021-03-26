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

#include <stdio.h>

void NewConnection(int sockfd, const dwater::net::InetAddress& peer_addr) {
    printf("NewConnection: accepted a new connection from %s\n", 
            peer_addr.ToIpPort().c_str());
    ::write(sockfd, "How are you?\n", 13);
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


