// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        timestamp.h
// Descripton:      时间戳的相关操作，比如时间戳的格式化，返回当前的时间戳，时间
// 戳的比较，时间戳之间的差值等等

#ifndef DWATER_SRC_BASE_TIMESTAMP_H
#define DWATER_SRC_BASE_TIMESTAMP_H

#include "copyable.h"
#include "types.h"

#include <boost/operators.hpp>

namespace dwater {

class Timestamp : public dwater::copyable,
                  public boost::equality_comparable<Timestamp>,
                  public boost::less_than_comparable<Timestamp> {
private:
    int64_t micro_seconds_since_epoch_;

public:
    static const int kmicro_seconds_per_second = 1000 * 1000;

public:

    Timestamp() : micro_seconds_since_epoch_(0) { }

    explicit Timestamp(int64_t micro_seconds_since_epoch)
        : micro_seconds_since_epoch_(micro_seconds_since_epoch){  }

    void swap(Timestamp& that) {
        std::swap(micro_seconds_since_epoch_, that.micro_seconds_since_epoch_);
    }

    string ToString() const;

    string ToFormattedString(bool show_micro_seconds = true) const;

    bool Valid() const { return micro_seconds_since_epoch_ > 0; }

    int64_t MicroSecondsSinceEpoch() const { return micro_seconds_since_epoch_; }

    time_t SecondSinceEpoch() const {
        return static_cast<time_t>(micro_seconds_since_epoch_ / kmicro_seconds_per_second); 
    }

    static Timestamp Now();

    static Timestamp Invalid() {
        return Timestamp();
    }

    static Timestamp FromUnixTime(time_t t) {
        return FromUnixTime(t, 0);
    }

    static Timestamp FromUnixTime(time_t t, int microseconds) {
        return Timestamp(static_cast<int64_t>(t) * kmicro_seconds_per_second + microseconds);
    }
};


inline bool operator<(Timestamp lhs, Timestamp rhs) {
    return lhs.MicroSecondsSinceEpoch() < rhs.MicroSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) {
    return lhs.MicroSecondsSinceEpoch() == rhs.MicroSecondsSinceEpoch();
}

inline double TimeDifference(Timestamp high, Timestamp low) {
    int64_t diff = high.MicroSecondsSinceEpoch() - low.MicroSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kmicro_seconds_per_second;
}

inline Timestamp AddTime(Timestamp timestap, double seconds) {
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kmicro_seconds_per_second);
    return Timestamp(timestap.MicroSecondsSinceEpoch() + delta);
}

} // namespace dwater

#endif // DWATER_SRC_BASE_TIMESTAMP_H
