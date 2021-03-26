// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.26
// Filename:        event_loop_thread_test.cc
// Descripton:       

#include "dwater/net/event_loop.h"
#include "dwater/net/event_loop_thread.h"

#include <iostream>

using namespace dwater;
using namespace dwater::net;

void ThreadFunc() {
    std::cout << "run in thread " << getpid() << " " << current_thread::Tid() << std::endl; 
}

int main() {
    std::cout << "main thead " << getpid() << " " << current_thread::Tid() << std::endl;
    EventLoopThread thead;
    EventLoop* loop = thead.StartLoop();
    loop->RunInLoop(ThreadFunc);
    sleep(1);
    loop->RunAfter(2, ThreadFunc);
    sleep(3);
    loop->Quit();
    std::cout << "main quit" << std::endl;

}


