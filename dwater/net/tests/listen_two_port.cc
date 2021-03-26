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

void NewConnection(int sockfd, const dwater::net::InetAddress& addr) {
    int port = addr.Port();
    printf("%d\n", port);
    if ( port == 9989 ) {
        ::write(sockfd, "Hello, I'm 9989\n", 16);
    } else if ( port == 9981 ) {
        ::write(sockfd, "Hello, I'm 9981\n", 16);
    } 
    dwater::net::socket::Close(sockfd);
}

int main() {
    printf("main(): pid = %d\n", getpid());
    dwater::net::InetAddress listen_addr(9981);
    dwater::net::InetAddress listen_addr2(9989);
    dwater::net::EventLoop loop;

    dwater::net::Acceptor acceptor(&loop, listen_addr,  false);
    dwater::net::Acceptor acceptor2(&loop, listen_addr2, false);
    acceptor.SetNewConnnectionCallback(NewConnection);
    acceptor2.SetNewConnnectionCallback(NewConnection);
    acceptor.Listen();
    acceptor2.Listen();
    loop.Loop();
}


