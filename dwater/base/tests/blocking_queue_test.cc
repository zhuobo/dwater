// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.20
// Filename:        blocking_queue_test.cc
// Descripton:      

#include "../blocking_queue.h"

#include "../thread.h"
#include "../count_down_latch.h"

#include <memory>
#include <string>
#include <vector>
#include <stdio.h>
#include <unistd.h>

class Test
{
 public:
  Test(int numThreads)
    : latch_(numThreads)
  {
    for (int i = 0; i < numThreads; ++i)
    {
      char name[32];
      snprintf(name, sizeof name, "work thread %d", i);
      threads_.emplace_back(new dwater::Thread(
            std::bind(&Test::threadFunc, this), dwater::string(name)));
    }
    for (auto& thr : threads_)
    {
      thr->Start();
    }
  }

  void run(int times)
  {
    printf("waiting for count down latch\n");
    latch_.Wait();
    printf("all threads started\n");
    for (int i = 0; i < times; ++i)
    {
      char buf[32];
      snprintf(buf, sizeof buf, "hello %d", i);
      queue_.Put(buf);
      printf("tid=%d, put data = %s, size = %zd\n", dwater::current_thread::Tid(), buf, queue_.Size());
    }
  }

  void joinAll()
  {
    for (size_t i = 0; i < threads_.size(); ++i)
    {
      queue_.Put("stop");
    }

    for (auto& thr : threads_)
    {
      thr->Join();
    }
  }

 private:

  void threadFunc()
  {
    printf("tid=%d, %s started\n",
           dwater::current_thread::Tid(),
           dwater::current_thread::Name());

    latch_.CountDown();
    bool running = true;
    while (running)
    {
      std::string d(queue_.Take());
      printf("tid=%d, get data = %s, size = %zd\n", dwater::current_thread::Tid(), d.c_str(), queue_.Size());
      running = (d != "stop");
    }

    printf("tid=%d, %s stopped\n",
           dwater::current_thread::Tid(),
           dwater::current_thread::Name());
  }

  dwater::BlockingQueue<std::string> queue_;
  dwater::CountDownLatch latch_;
  std::vector<std::unique_ptr<dwater::Thread>> threads_;
};

void testMove()
{
  dwater::BlockingQueue<std::unique_ptr<int>> queue;
  queue.Put(std::unique_ptr<int>(new int(42)));
  std::unique_ptr<int> x = queue.Take();
  printf("took %d\n", *x);
  *x = 123;
  queue.Put(std::move(x));
  std::unique_ptr<int> y = queue.Take();
  printf("took %d\n", *y);
}

int main()
{
  printf("pid=%d, tid=%d\n", ::getpid(), dwater::current_thread::Tid());
  Test t(5);
  t.run(100);
  t.joinAll();

  testMove();

  printf("number of created threads %d\n", dwater::Thread::NumCreated());
}



