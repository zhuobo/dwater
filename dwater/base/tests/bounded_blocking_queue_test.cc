// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.20
// Filename:        bounded_blocking_queue_test.h
// Descripton:       

#include "../bounded_blocking_queue.h"
#include "../condition.h"
#include "../thread.h"

#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <string>
class Test
{
 public:
  Test(int numThreads)
    : queue_(20),
      latch_(numThreads)
  {
    threads_.reserve(numThreads);
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

  dwater::BoundedBlockingQueue<std::string> queue_;
  dwater::CountDownLatch latch_;
  std::vector<std::unique_ptr<dwater::Thread>> threads_;
};

void testMove()
{
#if BOOST_VERSION >= 105500L
  dwater::BoundedBlockingQueue<std::unique_ptr<int>> queue(10);
  queue.Put(std::unique_ptr<int>(new int(42)));
  std::unique_ptr<int> x = queue.Take();
  printf("took %d\n", *x);
  *x = 123;
  queue.Put(std::move(x));
  std::unique_ptr<int> y;
  y = queue.Take();
  printf("took %d\n", *y);
#endif
}

int main()
{
  printf("pid=%d, tid=%d\n", ::getpid(), dwater::current_thread::Tid());
  testMove();
  Test t(5);
  t.run(100);
  t.joinAll();

  printf("number of created threads %d\n", dwater::Thread::NumCreated());
}
