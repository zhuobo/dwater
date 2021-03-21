// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        Timestemp_test.cc
// Descripton:      测试时间戳类 


#include "../timestamp.h"
#include <vector>
#include <stdio.h>

using dwater::Timestamp;

void passByConstReference(const Timestamp& x)
{
  printf("%s\n", x.ToString().c_str());
}

void passByValue(Timestamp x)
{
  printf("%s\n", x.ToString().c_str());
}

void benchmark()
{
  const int kNumber = 1000*1000;

  std::vector<Timestamp> stamps;
  stamps.reserve(kNumber);
  for (int i = 0; i < kNumber; ++i)
  {
    stamps.push_back(Timestamp::Now());
  }
  printf("%s\n", stamps.front().ToString().c_str());
  printf("%s\n", stamps.back().ToString().c_str());
  printf("%f\n", TimeDifference(stamps.back(), stamps.front()));

  int increments[100] = { 0 };
  int64_t start = stamps.front().MicroSecondsSinceEpoch();
  for (int i = 1; i < kNumber; ++i)
  {
    int64_t next = stamps[i].MicroSecondsSinceEpoch();
    int64_t inc = next - start;
    start = next;
    if (inc < 0)
    {
      printf("reverse!\n");
    }
    else if (inc < 100)
    {
      ++increments[inc];
    }
    else
    {
      printf("big gap %d\n", static_cast<int>(inc));
    }
  }

  for (int i = 0; i < 100; ++i)
  {
    printf("%2d: %d\n", i, increments[i]);
  }
}

int main()
{
  Timestamp Now(Timestamp::Now());
  printf("%s\n", Now.ToString().c_str());
  passByValue(Now);
  passByConstReference(Now);
  benchmark();
}
