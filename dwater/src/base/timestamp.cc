// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        timestamp.cc
// Descripton:      Timestamp类的实现

#include "timestamp.h"
#include <stdio.h>
#include <sys/time.h>

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>

using namespace dwater;

static_assert(sizeof(Timestamp) == sizeof(int64_t), "Timestamp should be same size as int64_t");

string Timestamp::ToString() const {
    char buf[32] = {0};
    int64_t seconds = micro_seconds_since_epoch_ / kmicro_seconds_per_second;
    int64_t microseconds = micro_seconds_since_epoch_ % kmicro_seconds_per_second;
    snprintf(buf, sizeof(buf), "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
    return buf;
}

string Timestamp::ToFormattedString(bool show_microsecons) const {
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(micro_seconds_since_epoch_ / kmicro_seconds_per_second);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    if ( show_microsecons ) {
        int microseconds = static_cast<int>(micro_seconds_since_epoch_ % kmicro_seconds_per_second);
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);
    } else {
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}

Timestamp Timestamp::Now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds * kmicro_seconds_per_second + tv.tv_usec);
}

