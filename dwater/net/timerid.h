// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.22
// Filename:        timerid.h
// Descripton:       

#ifndef DWATER_NET_TIMERID_H
#define DWATER_NET_TIMERID_H

#include "dwater/base/copyable.h"

#include <stdint.h> // for int64_t

namespace dwater {

namespace net {

class Timer;

class TimerId : public dwater::copyable {
public:
    TimerId(): timer_(nullptr), sequence_(0) {}

    TimerId(Timer* timer, int64_t sequence) : timer_(timer), sequence_(sequence) {}

    friend class TimerQueue;
private:
    Timer* timer_;
    int64_t sequence_;
}; // class Timerid

} // namespace net

} // namespace dwater

#endif // DWATER_NET_TIMERID_H 



