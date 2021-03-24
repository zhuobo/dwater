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

/// 
/// 定时器的一个整体，有两个成员一个是定时器，一个是这个定时器的序列号
/// 
class TimerId : public dwater::copyable {
public:
    TimerId(): timer_(nullptr), sequence_(0) {}

    TimerId(Timer* timer, int64_t sequence) : timer_(timer), sequence_(sequence) {}

    friend class TimerQueue;
private:
    Timer* timer_;
    int64_t sequence_; // 定时器序列号，由原子数生成
}; // class Timerid

} // namespace net

} // namespace dwater

#endif // DWATER_NET_TIMERID_H 



