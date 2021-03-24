// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        
// Descripton:       


#include "dwater/base/logging.h"
#include "dwater/net/channel.h"
#include "dwater/net/event_loop.h"

#include <functional>
#include <map>

#include <stdio.h>
#include <unistd.h>
#include <sys/timerfd.h>

using namespace dwater;
using namespace dwater::net;

void print(const char* msg)
{
  static std::map<const char*, Timestamp> lasts;
  Timestamp& last = lasts[msg];
  Timestamp now = Timestamp::Now();
  printf("%s tid %d %s delay %f\n", now.ToString().c_str(), current_thread::Tid(),
         msg, TimeDifference(now, last));
  last = now;
}

namespace dwater
{
namespace net
{
namespace detail
{
int CreateTimerfd();
void ReadTimerfd(int timerfd, Timestamp now);
}
}
}

// Use relative time, immunized to wall clock changes.
class PeriodicTimer
{
 public:
  PeriodicTimer(EventLoop* loop, double interval, const TimerCallback& cb)
    : loop_(loop),
      timerfd_(dwater::net::detail::CreateTimerfd()),
      timerfdChannel_(loop, timerfd_),
      interval_(interval),
      cb_(cb)
  {
    timerfdChannel_.SetReadCallback(
        std::bind(&PeriodicTimer::handleRead, this));
    timerfdChannel_.EnableReading();
  }

  void start()
  {
    struct itimerspec spec;
    MemZero(&spec, sizeof spec);
    spec.it_interval = toTimeSpec(interval_);
    spec.it_value = spec.it_interval;
    int ret = ::timerfd_settime(timerfd_, 0 /* relative timer */, &spec, NULL);
    if (ret)
    {
      LOG_SYSERR << "timerfd_settime()";
    }
  }

  ~PeriodicTimer()
  {
    timerfdChannel_.DisableAll();
    timerfdChannel_.Remove();
    ::close(timerfd_);
  }

 private:
  void handleRead()
  {
    loop_->AssertInLoopThread();
    dwater::net::detail::ReadTimerfd(timerfd_, Timestamp::Now());
    if (cb_)
      cb_();
  }

  static struct timespec toTimeSpec(double seconds)
  {
    struct timespec ts;
    MemZero(&ts, sizeof ts);
    const int64_t kNanoSecondsPerSecond = 1000000000;
    const int kMinInterval = 100000;
    int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);
    if (nanoseconds < kMinInterval)
      nanoseconds = kMinInterval;
    ts.tv_sec = static_cast<time_t>(nanoseconds / kNanoSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(nanoseconds % kNanoSecondsPerSecond);
    return ts;
  }

  EventLoop* loop_;
  const int timerfd_;
  Channel timerfdChannel_;
  const double interval_; // in seconds
  TimerCallback cb_;
};

int main()
{
  LOG_INFO << "pid = " << getpid() << ", tid = " <<current_thread::Tid()
           << " Try adjusting the wall clock, see what happens.";
  EventLoop loop;
  PeriodicTimer timer(&loop, 1, std::bind(print, "PeriodicTimer"));
  timer.start();
  loop.RunEvery(1, std::bind(print, "EventLoop::runEvery"));
  loop.Loop();
}


