// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.26
// Filename:        event_loop_thread_pool_test.cc
// Descripton:       

#include "dwater/net/event_loop_thread_pool.h"
#include "dwater/net/event_loop.h"
#include "dwater/base/thread.h"

#include <stdio.h>
#include <unistd.h>

using namespace dwater;
using namespace dwater::net;

void print(EventLoop* p = NULL)
{
  printf("main(): pid = %d, tid = %d, loop = %p\n",
         getpid(), current_thread::Tid(), p);
}

void init(EventLoop* p)
{
  printf("init(): pid = %d, tid = %d, loop = %p\n",
         getpid(), current_thread::Tid(), p);
}

int main()
{
  print();

  EventLoop loop;
  loop.RunAfter(11, std::bind(&EventLoop::Quit, &loop));

  {
    printf("Single thread %p:\n", &loop);
    EventLoopThreadPool model(&loop, "single");
    model.SetThreadNum(0);
    model.Start(init);
    assert(model.GetNextLoop() == &loop);
    assert(model.GetNextLoop() == &loop);
    assert(model.GetNextLoop() == &loop);
  }

  {
    printf("Another thread:\n");
    EventLoopThreadPool model(&loop, "another");
    model.SetThreadNum(1);
    model.Start(init);
    EventLoop* nextLoop = model.GetNextLoop();
    nextLoop->RunAfter(2, std::bind(print, nextLoop));
    assert(nextLoop != &loop);
    assert(nextLoop == model.GetNextLoop());
    assert(nextLoop == model.GetNextLoop());
    ::sleep(3);
  }

  {
    printf("Three threads:\n");
    EventLoopThreadPool model(&loop, "three");
    model.SetThreadNum(3);
    model.Start(init);
    EventLoop* nextLoop = model.GetNextLoop();
    nextLoop->RunInLoop(std::bind(print, nextLoop));
    assert(nextLoop != &loop);
    assert(nextLoop != model.GetNextLoop());
    assert(nextLoop != model.GetNextLoop());
    assert(nextLoop == model.GetNextLoop());
  }

  loop.Loop();
}
