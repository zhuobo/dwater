// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.23
// Filename:        timer.cc
// Descripton:       


#include "dwater/net/timer.h"

using namespace dwater;
using namespace dwater::net;

AtomicInt64 Timer::s_num_created_;

void Timer::Restart(Timestamp now) {
    if ( repeat_ ) {
        expiration_ = AddTime(now, interval_);
    } else {
        expiration_ = Timestamp::Invalid();
    }
}



