// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.21
// Filename:        async_logging_test.cc
// Descripton:      异步日志测试

#include "../async_logging.h"
#include "../logging.h"
#include "../timestamp.h"

#include <unistd.h>
#include <sys/resource.h>
#include <stdio.h>

using namespace dwater;

off_t kroll_size = 500 * 1000 * 100;

dwater::AsyncLogging* g_async_log = NULL;

void asyncOutput(const char* msg, int len)
{
  g_async_log->Append(msg, len);
}

void bench(bool longLog)
{
  Logger::SetOutput(asyncOutput);

  int cnt = 0;
  const int kBatch = 1000;
  string empty = " ";
  string longStr(3000, 'X');
  longStr += " ";

  for (int t = 0; t < 30; ++t)
  {
    Timestamp start = Timestamp::Now();
    for (int i = 0; i < kBatch; ++i)
    {
      LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
               << (longLog ? longStr : empty)
               << cnt;
      ++cnt;
    }
    Timestamp end = Timestamp::Now();
    printf("%f\n", TimeDifference(end, start)*1000000/kBatch);
    struct timespec ts = { 0, 500*1000*1000 };
    nanosleep(&ts, NULL);
  }
}

int main(int argc, char* argv[])
{
  {
    // set max virtual memory to 2GB.
    size_t kOneGB = 1000*1024*1024;
    rlimit rl = { 2*kOneGB, 2*kOneGB };
    setrlimit(RLIMIT_AS, &rl);
  }

  printf("pid = %d\n", getpid());

  char name[256] = { '\0' };
  strncpy(name, argv[0], sizeof name - 1);
  AsyncLogging log(::basename(name), kroll_size);
  log.Start();
  g_async_log = &log;

  bool longLog = argc > 1;
  bench(longLog);
}

