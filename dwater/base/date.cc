// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        date.cc
// Descripton:      Date的实现 

#include "dwater/base/date.h"

#include <stdio.h> // for snprintf
#include <time.h> // for struct tm

namespace dwater {

namespace detail {

char require_32_bit_integer_at_least[sizeof(int) >= sizeof(int32_t) ? 1 : -1];

// 获取julian day的天数
int GetJulianDayNumber(int year, int month, int day) {
    (void) require_32_bit_integer_at_least;
    int a = (14 - month) / 12;
    int y = year + 4800 - a;
    int m = month + 12 * a - 3;
    return day + (153 * m + 2) / 5 + y * 365 + y / 4 - y / 100 + y / 400 - 32045;
}

struct Date::Year_Month_Day GetYearMonthDay(int julian_day_number) {
    int a = julian_day_number + 32044;
    int b = (4 * a + 3) / 146097;
    int c = a - ((b * 146097) / 4);
    int d = (4 * c + 3) / 1461;
    int e = c - ((1461 * d) / 4);
    int m = (5 * e + 2) / 153;
    Date::Year_Month_Day ymd;
    ymd.day = e - ((153 * m + 2) / 5) + 1;
    ymd.month = m + 3 - 12 * (m / 10);
    ymd.year = b * 100 + d - 4800 + (m / 10);
    return ymd;
}

} // namespace detail

const int Date::kjulian_day_of_19700101 = detail::GetJulianDayNumber(1970, 1, 1);
} // namespace dwater


using namespace dwater;
using namespace dwater::detail;

Date::Date(int y, int m, int d) : julian_day_number_(GetJulianDayNumber(y, m, d)) {}

Date::Date(const struct tm& t)
    : julian_day_number_(GetJulianDayNumber(
    t.tm_year + 1900,
    t.tm_mon + 1,
    t.tm_mday
    )) {
}

string Date::ToIsoString() const {
    char buf[32];
    Year_Month_Day ymd(YearMonthDay());
    snprintf(buf, sizeof buf, "%4d-%02d-%02d", ymd.year, ymd.month, ymd.day);
    return buf;
}

Date::Year_Month_Day Date::YearMonthDay() const {
    return GetYearMonthDay(julian_day_number_);
}
