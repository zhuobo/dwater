// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        date.h
// Descripton:       

#ifndef DWATER_SRC_BASE_DATE_H
#define DWATER_SRC_BASE_DATE_H

#include "copyable.h"
#include "types.h"

struct tm;

namespace dwater {

class Date : public dwater::copyable {
public:
    struct Year_Month_Day {
        int year;
        int month;
        int day;
    };

    static const int kdays_per_week = 7;
    static const int kjulian_day_of_19700101;

    /// 
    /// invalid Date
    Date() : julian_day_number_(0) {}

    Date(int year, int month, int day);

    explicit Date(int julian_day_num) : julian_day_number_(julian_day_num) {}

    explicit Date(const struct tm&);

    void Swap(Date& that) {
        std::swap(julian_day_number_, that.julian_day_number_);
    }

    bool Valid() const {
        return julian_day_number_ > 0;
    }

    string ToIsoString() const;

    struct Year_Month_Day YearMonthDay() const;

    int Year() const {
        return YearMonthDay().year;
    }

    int Month() const {
        return YearMonthDay().month;
    }

    int Day() const {
        return YearMonthDay().day;
    }

    int WeekDay() const {
        return (julian_day_number_ + 1) % kdays_per_week;
    }

    int JulianDayNumber() const { 
        return julian_day_number_;
    }

private:
    int julian_day_number_;


}; // class Date

inline bool operator<(Date x, Date y) {
    return x.JulianDayNumber() < y.JulianDayNumber();
}

inline bool operator==(Date x, Date y) {
    return x.JulianDayNumber() == y.JulianDayNumber();
}

} // namespace dwater

#endif // DWATER_SRC_BASE_DATE_H

