///////////////////////////////////////////////////////////////////////////////
// The implement of class LogStream
// ////////////////////////////////////////////////////////////////////////////
#include "dwater/base/log_stream.h"

#include <algorithm>
#include <limits>
#include <type_traits>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>

using namespace dwater;
using namespace dwater::detail;

namespace dwater {

namespace detail {
const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
static_assert(sizeof(digits) == 20, "wrong number of digits");

const char digits_hex[] = "0123456789ABCDEF";
static_assert(sizeof(digits_hex) == 17, "wrong number of digits_hex");

// Integer to String
template<typename T>
size_t Convert(char buf[], T value) {
    T i = value;
    char *p = buf;

    do {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while ( i != 0 );
    if ( value < 0 ) {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);
    return p - buf;
}

size_t ConvertHex(char buf[], uintptr_t value) {
    uintptr_t i = value;
    char* p = buf;

    do {
        int lsd = static_cast<int>(i % 16);
        i /= 16;
        *p++ = digits_hex[lsd];
    } while ( i != 0 );
    *p++ = '\0';
    std::reverse(buf, p);
    return p - buf;
}

template class FixedBuffer<k_small_buffer>;
template class FixedBuffer<k_large_buffer>;

} // namespace detail


std::string FormatSI(int64_t s)
{
  double n = static_cast<double>(s);
  char buf[64];
  if (s < 1000)
    snprintf(buf, sizeof(buf), "%" PRId64, s);
  else if (s < 9995)
    snprintf(buf, sizeof(buf), "%.2fk", n/1e3);
  else if (s < 99950)
    snprintf(buf, sizeof(buf), "%.1fk", n/1e3);
  else if (s < 999500)
    snprintf(buf, sizeof(buf), "%.0fk", n/1e3);
  else if (s < 9995000)
    snprintf(buf, sizeof(buf), "%.2fM", n/1e6);
  else if (s < 99950000)
    snprintf(buf, sizeof(buf), "%.1fM", n/1e6);
  else if (s < 999500000)
    snprintf(buf, sizeof(buf), "%.0fM", n/1e6);
  else if (s < 9995000000)
    snprintf(buf, sizeof(buf), "%.2fG", n/1e9);
  else if (s < 99950000000)
    snprintf(buf, sizeof(buf), "%.1fG", n/1e9);
  else if (s < 999500000000)
    snprintf(buf, sizeof(buf), "%.0fG", n/1e9);
  else if (s < 9995000000000)
    snprintf(buf, sizeof(buf), "%.2fT", n/1e12);
  else if (s < 99950000000000)
    snprintf(buf, sizeof(buf), "%.1fT", n/1e12);
  else if (s < 999500000000000)
    snprintf(buf, sizeof(buf), "%.0fT", n/1e12);
  else if (s < 9995000000000000)
    snprintf(buf, sizeof(buf), "%.2fP", n/1e15);
  else if (s < 99950000000000000)
    snprintf(buf, sizeof(buf), "%.1fP", n/1e15);
  else if (s < 999500000000000000)
    snprintf(buf, sizeof(buf), "%.0fP", n/1e15);
  else
    snprintf(buf, sizeof(buf), "%.2fE", n/1e18);
  return buf;
}

/*
 [0, 1023]
 [1.00Ki, 9.99Ki]
 [10.0Ki, 99.9Ki]
 [ 100Ki, 1023Ki]
 [1.00Mi, 9.99Mi]
*/
std::string FormatIEC(int64_t s)
{
  double n = static_cast<double>(s);
  char buf[64];
  const double Ki = 1024.0;
  const double Mi = Ki * 1024.0;
  const double Gi = Mi * 1024.0;
  const double Ti = Gi * 1024.0;
  const double Pi = Ti * 1024.0;
  const double Ei = Pi * 1024.0;

  if (n < Ki)
    snprintf(buf, sizeof buf, "%" PRId64, s);
  else if (n < Ki*9.995)
    snprintf(buf, sizeof buf, "%.2fKi", n / Ki);
  else if (n < Ki*99.95)
    snprintf(buf, sizeof buf, "%.1fKi", n / Ki);
  else if (n < Ki*1023.5)
    snprintf(buf, sizeof buf, "%.0fKi", n / Ki);

  else if (n < Mi*9.995)
    snprintf(buf, sizeof buf, "%.2fMi", n / Mi);
  else if (n < Mi*99.95)
    snprintf(buf, sizeof buf, "%.1fMi", n / Mi);
  else if (n < Mi*1023.5)
    snprintf(buf, sizeof buf, "%.0fMi", n / Mi);

  else if (n < Gi*9.995)
    snprintf(buf, sizeof buf, "%.2fGi", n / Gi);
  else if (n < Gi*99.95)
    snprintf(buf, sizeof buf, "%.1fGi", n / Gi);
  else if (n < Gi*1023.5)
    snprintf(buf, sizeof buf, "%.0fGi", n / Gi);

  else if (n < Ti*9.995)
    snprintf(buf, sizeof buf, "%.2fTi", n / Ti);
  else if (n < Ti*99.95)
    snprintf(buf, sizeof buf, "%.1fTi", n / Ti);
  else if (n < Ti*1023.5)
    snprintf(buf, sizeof buf, "%.0fTi", n / Ti);

  else if (n < Pi*9.995)
    snprintf(buf, sizeof buf, "%.2fPi", n / Pi);
  else if (n < Pi*99.95)
    snprintf(buf, sizeof buf, "%.1fPi", n / Pi);
  else if (n < Pi*1023.5)
    snprintf(buf, sizeof buf, "%.0fPi", n / Pi);

  else if (n < Ei*9.995)
    snprintf(buf, sizeof buf, "%.2fEi", n / Ei );
  else
    snprintf(buf, sizeof buf, "%.1fEi", n / Ei );
  return buf;
}
} // namespace dwater

// impletement 
template<int SIZE>
const char* FixedBuffer<SIZE>::DebugString() {
    *curr_ = '\0';
    return data_;
}


void LogStream::StaticCheck() {
    static_assert(k_max_numeric_size - 10 > std::numeric_limits<double>::digits10,
            "k_max_numeric_size is large enough");
    static_assert(k_max_numeric_size - 10 > std::numeric_limits<long double>::digits10,
            "k_max_numeric_size is large enough");
    static_assert(k_max_numeric_size - 10 > std::numeric_limits<long>::digits10,
            "k_max_numeric_size is large enough");
    static_assert(k_max_numeric_size - 10 > std::numeric_limits<long long>::digits10,
            "k_max_numeric_size is large enough");
}

template<typename T>
void LogStream::FormatInteger(T v) {
    if ( buffer_.Avail() >= k_max_numeric_size ) {
        size_t len = Convert(buffer_.Current(), v);
        buffer_.Add(len);
    }
}

LogStream& LogStream::operator<<(short v) {
    *this << static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned short  v) {
    *this << static_cast<unsigned int>(v);
    return *this;
}

LogStream& LogStream::operator<<(int v) {
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int v) {
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long v) {
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long v) {
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long long v) {
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long v) {
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(const void* p) {
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    if ( buffer_.Avail() >= k_max_numeric_size ) {
        char* buf = buffer_.Current();
        buf[0] = '0';
        buf[1] = 'x';
        size_t len = ConvertHex(buf + 2, v);
        buffer_.Add(len + 2);
    }
    return *this;
}

LogStream& LogStream::operator<<(double v) {
    if ( buffer_.Avail() >= k_max_numeric_size ) {
        int len = snprintf(buffer_.Current(), k_max_numeric_size, "%.12g", v);
        buffer_.Add(len);
    }
    return *this;
}


template<typename T>
Format::Format(const char* fmt, T val) {
    static_assert(std::is_arithmetic<T>::value == true, "Must be arithmetic type");
    length_ = snprintf(buf_, sizeof(buf_), fmt, val);
    assert(static_cast<size_t>(length_) < sizeof(buf_));
}

template Format::Format(const char* fmt, char);

template Format::Format(const char* fmt, short);
template Format::Format(const char* fmt, unsigned short);
template Format::Format(const char* fmt, int);
template Format::Format(const char* fmt, unsigned int);
template Format::Format(const char* fmt, long);
template Format::Format(const char* fmt, unsigned long);
template Format::Format(const char* fmt, long long);
template Format::Format(const char* fmt, unsigned long long);

template Format::Format(const char* fmt, float);
template Format::Format(const char* fmt, double);
