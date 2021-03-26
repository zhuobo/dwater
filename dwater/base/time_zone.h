// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        time_zone.h
// Descripton:      封装关于时区的操作，主要用于时区以及夏令时的转换

#ifndef DWATER_SRC_BASE_TIME_ZONE_H
#define DWATER_SRC_BASE_TIME_ZONE_H

#include "dwater/base/copyable.h"

#include <memory>
#include <time.h>

namespace dwater {

class TimeZone : public copyable {
public:
    struct Data;

    explicit TimeZone(const char* zonefile);

    TimeZone(int east_of_UTC, const char* tzname);

    TimeZone() = default;

    bool valid() const {
        return static_cast<bool>(data_);
    }

    // 转化为本地时间
    struct tm ToLocalTime(time_t  seconds_since_epoch) const;
    // 返回秒数 
    time_t FromLocalTime(const struct tm&) const;

    static struct tm ToUtcTime(time_t seconds_since_epoch, bool yday = false);

    static time_t FromUtcTime(const struct tm&);

    static time_t FromUtcTime(int year, int month, int day, int hour, int minute, int second);

private:
    std::shared_ptr<Data> data_;
}; // class TimeZone

} // namespace dwater

#endif // DWATER_SRC_BASE_TIME_ZONE_H
