// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        time_zone.cc
// Descripton:      时区操作

#include "dwater/base/time_zone.h"
#include "dwater/base/noncopable.h"
#include "dwater/base/date.h"

#include <algorithm>
#include <string>
#include <vector>
#include <stdexcept>

#include <assert.h>
#include <endian.h>
#include <stdint.h>
#include <stdio.h>

namespace dwater {

namespace detail {

// 时间
struct Transition {
    time_t gmttime;
    time_t localtime;
    int    localtime_index;

    Transition(time_t t, time_t l, int local_index)
        : gmttime(t), localtime(l), localtime_index(local_index) {}
}; // struct Transition

struct Comp {
    bool compare_gmt;

    Comp(bool gmt) : compare_gmt(gmt) {}

    bool operator()(const Transition& lhs, const Transition& rhs) const {
        if ( compare_gmt ) {
            return lhs.gmttime < rhs.gmttime;
        } else {
            return lhs.localtime < rhs.localtime;
        }
    }


    bool Equal(const Transition& lhs, const Transition& rhs) const {
        if ( compare_gmt ) {
            return lhs.gmttime < rhs.gmttime;
        } else {
            return lhs.localtime < rhs.localtime;
        }
    }
}; // struct Comp

struct Localtime {
    time_t gmt_offset;
    bool   is_dst;
    int    arrb_index;

    Localtime(time_t offset, bool dst, int arrb)
        : gmt_offset(offset), is_dst(dst), arrb_index(arrb) {}
}; // struct  Localtime

inline void FillHMS(unsigned seconds, struct tm* utc) {
    utc->tm_sec = seconds % 60;
    unsigned minutes = seconds / 60;
    utc->tm_min = minutes % 60;
    utc->tm_hour = minutes / 60;
}

} // namespace detail

const int kseconds_per_day = 24 * 60 * 60;

} // namespace dwater

using namespace dwater;
using namespace std;

struct TimeZone::Data {
    vector<detail::Transition> transitions;
    vector<detail::Localtime> localtimes;
    vector<string> names;
    string abbreviation;
};







namespace dwater {

namespace detail {

class File : noncopyable {
public:
    File(const char* file) : fp_(::fopen(file, "rb")) {}

    ~File() {
        if ( fp_ ) {
            ::fclose(fp_);
        }
    }

    bool Valid() const {
        return fp_;
    }

    string ReadBytes(int n) {
        char buf[n];
        ssize_t nr = ::fread(buf, 1, n, fp_);
        if ( nr != n ) {
            throw logic_error("no enough data");
        }
        return string(buf, n);
    }

    int32_t ReadInt32() {
        int32_t x = 0;
        ssize_t nr = ::fread(&x, 1, sizeof(int32_t), fp_);
        if ( nr != sizeof(int32_t) ) {
            throw logic_error("bad int32_t data");
        }
        return be32toh(x);
    }

    uint8_t ReadUInt8() {
        uint8_t x = 0;
        ssize_t nr = ::fread(&x, 1, sizeof(uint8_t), fp_);
        if ( nr != sizeof(uint8_t) ) {
            throw logic_error("bad uint8_t data");
        }
        return x;
    }
private:
    FILE* fp_;
}; // class File

bool ReadTimeZoneFile(const char* zone_file, struct TimeZone::Data* data) {
    File f(zone_file);
    if ( f.Valid() ) {
        try {
            string head = f.ReadBytes(4);
            if ( head != "TZif" ) {
                throw logic_error("bad head");
            }
            string version = f.ReadBytes(1);
            f.ReadBytes(15);

            int32_t isgmtcnt = f.ReadInt32();
            int32_t isstdcnt = f.ReadInt32();
            int32_t leapcnt = f.ReadInt32();
            int32_t timecnt = f.ReadInt32();
            int32_t typecnt = f.ReadInt32();
            int32_t charcnt = f.ReadInt32();

            vector<int32_t> trans;
            vector<int> localtimes;
            trans.reserve(timecnt);
            for (int i = 0; i < timecnt; ++i) {
                trans.push_back(f.ReadInt32());
            }

            for (int i = 0; i < timecnt; ++i) {
                uint8_t local = f.ReadUInt8();
                localtimes.push_back(local);
            }

            for (int i = 0; i < typecnt; ++i) {
                int32_t gmtoff = f.ReadInt32();
                uint8_t isdst = f.ReadUInt8();
                uint8_t abbrind = f.ReadUInt8();

                data->localtimes.push_back(Localtime(gmtoff, isdst, abbrind));
            }

            for (int i = 0; i < timecnt; ++i) {
                int localIdx = localtimes[i];
                time_t localtime = trans[i] + data->localtimes[localIdx].gmt_offset;
                data->transitions.push_back(Transition(trans[i], localtime, localIdx));
            }

            data->abbreviation = f.ReadBytes(charcnt);

            // leapcnt
            for (int i = 0; i < leapcnt; ++i) {
                // int32_t leaptime = f.ReadInt32();
                // int32_t cumleap = f.ReadInt32();
            }
      	// FIXME
            (void) isstdcnt;
            (void) isgmtcnt;
        }
        catch (logic_error& e) {
            fprintf(stderr, "%s\n", e.what());
        }
  }
  return true;
}


const Localtime* FindLocaltime(const TimeZone::Data& data, Transition sentry, Comp comp) {
    const Localtime* local  = NULL;
    if ( data.transitions.empty() || comp(sentry, data.transitions.front()) ) {
        local = &data.localtimes.front();
    } else {
        vector<Transition>::const_iterator iter = lower_bound(data.transitions.begin(),
                data.transitions.end(), sentry, comp);
        if ( iter != data.transitions.end() ) {
            if ( !comp.Equal(sentry, *iter) ) {
                assert(iter != data.transitions.begin());
                --iter;
            }
            local = &data.localtimes[iter->localtime_index];
        } else {
            local = &data.localtimes[data.transitions.back().localtime_index];
        }
    }
    return local;
}

} // namesapce detail

} // namespace dwater


TimeZone::TimeZone(const char* zone_file) : data_(new TimeZone::Data) {
    if ( !detail::ReadTimeZoneFile(zone_file, data_.get()) ) {
        data_.reset();
    }
}

TimeZone::TimeZone(int east_of_utc, const char* name) : data_(new TimeZone::Data) {
    data_->localtimes.push_back(detail::Localtime(east_of_utc, false, 0));
    data_->abbreviation = name;
}

struct tm TimeZone::ToLocalTime(time_t seconds) const {
    struct tm local_time;
    dwater::MemZero(&local_time, sizeof(local_time));
    assert(data_ != NULL);
    const Data& data(*data_);

    detail::Transition sentry(seconds, 0, 0);
    const detail::Localtime* local = FindLocaltime(data, sentry, detail::Comp(true));

    if ( local ) {
        time_t local_seonds = seconds + local->gmt_offset;
        ::gmtime_r(&local_seonds, &local_time);
        local_time.tm_isdst = local->is_dst;
        local_time.tm_gmtoff = local->gmt_offset;
        local_time.tm_zone = &data.abbreviation[local->arrb_index];
    }

    return local_time;
}

time_t TimeZone::FromLocalTime(const struct tm& localtime) const {
    assert(data_ != NULL);
    const Data& data(*data_);

    struct tm temp = localtime;
    time_t seconds = ::timegm(&temp);
    detail::Transition sentry(0, seconds, 0);
    const detail::Localtime* local = FindLocaltime(data, sentry, detail::Comp(false));

    if ( localtime.tm_isdst ) {
        struct tm try_tm = ToLocalTime(seconds - local->gmt_offset);
        if ( !try_tm.tm_isdst
            && try_tm.tm_hour == localtime.tm_hour
            && try_tm.tm_min == localtime.tm_min) {
            seconds -= 60 * 60;
        }
    }
    return seconds - local->gmt_offset;
}

struct tm TimeZone::ToUtcTime(time_t seconds_since_epoch, bool yday) {
    struct tm utc;
    MemZero(&utc, sizeof(utc));
    utc.tm_zone = "GMT";
    int seconds = static_cast<int>(seconds_since_epoch % kseconds_per_day);
    int days = static_cast<int>(seconds_since_epoch / kseconds_per_day);
    if ( seconds < 0 ) {
        seconds += kseconds_per_day;
        --days;
    }

    detail::FillHMS(seconds, &utc);
    Date date(days + Date::kjulian_day_of_19700101);
    Date::Year_Month_Day ymd = date.YearMonthDay();
    utc.tm_year  = ymd.year - 1900;
    utc.tm_mon = ymd.month - 1;
    utc.tm_mday = ymd.day;
    utc.tm_wday = date.WeekDay();

    if ( yday ) {
        Date start_of_year(ymd.year, 1, 1);
        utc.tm_yday = date.JulianDayNumber() - start_of_year.JulianDayNumber();
    }
    return utc;
}

time_t TimeZone::FromUtcTime(const struct tm& utc) {
    return FromUtcTime(utc.tm_year + 1900, utc.tm_mon + 1, utc.tm_mday,
            utc.tm_hour, utc.tm_min, utc.tm_sec);
}

time_t TimeZone::FromUtcTime(int year, int month, int day,
                             int hour, int minute, int seconds) {
    Date date(year, month, day);
    int seconds_in_day = hour * 3600 + minute * 60 + seconds;
    time_t days = date.JulianDayNumber() - Date::kjulian_day_of_19700101;
    return days * kseconds_per_day + seconds_in_day;
}

