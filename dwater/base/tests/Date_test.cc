// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        Date_test.cc
// DescripTon:      testing class Date 


#include "../date.h"
#include <assert.h>
#include <stdio.h>

using dwater::Date;

const int kMonthsOfYear = 12;

int isLeapYear(int year)
{
  if (year % 400 == 0)
    return 1;
  else if (year % 100 == 0)
    return 0;
  else if (year % 4 == 0)
    return 1;
  else
    return 0;
}

int daysOfMonth(int year, int month)
{
  static int days[2][kMonthsOfYear+1] =
  {
    { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
  };
  return days[isLeapYear(year)][month];
}

void passByConstReference(const Date& x)
{
  printf("%s\n", x.ToIsoString().c_str());
}

void passByValue(Date x)
{
  printf("%s\n", x.ToIsoString().c_str());
}

int main()
{
  time_t now = time(NULL);
  struct tm t1 = *gmtime(&now);
  struct tm t2 = *localtime(&now);
  Date someDay(2008, 9, 10);
  printf("%s\n", someDay.ToIsoString().c_str());
  passByValue(someDay);
  passByConstReference(someDay);
  Date TodayUtc(t1);
  printf("%s\n", TodayUtc.ToIsoString().c_str());
  Date TodayLocal(t2);
  printf("%s\n", TodayLocal.ToIsoString().c_str());

  int julianDayNumber = 2415021;
  int weekDay = 1; // Monday

  for (int year = 1900; year < 2500; ++year)
  {
    assert(Date(year, 3, 1).JulianDayNumber() - Date(year, 2, 29).JulianDayNumber()
           == isLeapYear(year));
    for (int month = 1; month <= kMonthsOfYear; ++month)
    {
      for (int day = 1; day <= daysOfMonth(year, month); ++day)
      {
        Date d(year, month, day);
        // printf("%s %d\n", d.ToString().c_str(), d.WeekDay());
        assert(year == d.Year());
        assert(month == d.Month());
        assert(day == d.Day());
        assert(weekDay == d.WeekDay());
        assert(julianDayNumber == d.JulianDayNumber());

        Date d2(julianDayNumber);
        assert(year == d2.Year());
        assert(month == d2.Month());
        assert(day == d2.Day());
        assert(weekDay == d2.WeekDay());
        assert(julianDayNumber == d2.JulianDayNumber());

        ++julianDayNumber;
        weekDay = (weekDay+1) % 7;
      }
    }
  }
  printf("All passed.\n");
}




