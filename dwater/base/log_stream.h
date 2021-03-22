///////////////////////// log_stream  /////////////////////////////////////////
// Format output, overload << stream operation symbol
// ///////////////////////////////////////////////////////////////////////////

#ifndef DWATER_SRC_BASE_LOG_STREAM_H
#define DWATER_SRC_BASE_LOG_STREAM_H

#include "dwater/base/noncopable.h"
#include "dwater/base/string_piece.h"
#include "dwater/base/types.h"

#include <assert.h>
#include <string.h>

namespace dwater {

namespace detail {

const int k_small_buffer = 4000;
const int k_large_buffer = 4000 * 1000;

// FixedBuffer is used to store log imformation by LogStream
template<int SIZE>
class FixedBuffer : noncopyable {
private:
    char  data_[SIZE];
    char* curr_;

private:
    const char* end() const {
        return data_ + sizeof(data_);
    }
    void (*_cookie)();

public:
    FixedBuffer() : curr_(data_) {
    }

    ~FixedBuffer() {
    }

    // 如果可以全部写入就直接写入
    void Append(const char* buf, size_t len) {
        if ( dwater::implicit_cast<size_t>(Avail()) > len ) {
            memcpy(curr_, buf, len);
            curr_ += len;
        }
    }

    const char* Data() const {
        return data_;
    }

    int Length() const {
        return static_cast<int>(curr_ - data_);
    }

    char* Current() {
        return curr_;
    }

    // 返回可以写的长度
    int Avail() const { 
        return static_cast<int>(end() - data_);
    }

    void Add(size_t len) {
        curr_ += len;
    }

    void Reset() {
        curr_ = data_;
    }

    void Bzero() {
        dwater::MemZero(data_, sizeof(data_));
    }

    const char* DebugString();

    void SetCookie(void (*cookie)()) {
        _cookie = cookie;
    }

    string ToString() const { 
        return string(data_, Length());
    }

    StringPiece ToStringPiece() const {
        return StringPiece(data_, Length());
    }
}; // class FixedBuffer
    
} // namespace detail

class LogStream : noncopyable {
public:
    typedef detail::FixedBuffer<detail::k_small_buffer> Buffer;

private:
    Buffer buffer_;
    static const int k_max_numeric_size = 32;

private:
    template<typename T>
    void FormatInteger(T);

    void StaticCheck();

public:
    LogStream& operator<<(bool v) {
        buffer_.Append(v ? "1" : "0", 1);
        return *this;
    }

    // impletements is in log_steam.cc
    LogStream& operator<<(short);
    LogStream& operator<<(unsigned short);
    LogStream& operator<<(int);
    LogStream& operator<<(unsigned int);
    LogStream& operator<<(long);
    LogStream& operator<<(unsigned long);
    LogStream& operator<<(long long);
    LogStream& operator<<(unsigned long long);

    LogStream& operator<<(const void*);

    LogStream& operator<<(float v) {
        *this << static_cast<double>(v);
        return *this;
    }

    LogStream& operator<<(double);

    LogStream& operator<<(char v) {
        buffer_.Append(&v, 1);
        return *this;
    }

    LogStream& operator<<(const char* str) {
        if ( str ) {
            buffer_.Append(str, strlen(str));
        } else {
            buffer_.Append("(null)", 6);
        }
        return *this;
    }

    LogStream& operator<<(const unsigned char* str) {
        return operator<<(reinterpret_cast<const char*>(str));
    }
    
    LogStream& operator<<(const string& v) {
        buffer_.Append(v.c_str(), v.size());
        return *this;
    }

    LogStream& operator<<(const StringPiece& v) {
        buffer_.Append(v.Data(), v.Size());
        return *this;
    }

    LogStream& operator<<(const Buffer& v) {
        *this << v.ToStringPiece();
        return *this;
    }

    void Append(const char* data, int len) {
        buffer_.Append(data, len);
    }

    const Buffer& GetBuffer() const {
        return buffer_;
    }

    void ResetBuffer() {
        buffer_.Reset();
    }
};  // LogStream

class Format {
private:
    char buf_[32];
    int  length_;

public:
    template<typename T>
    Format(const char* format, T val);

    const char* Data() const {
        return buf_;
    }

    int Length() const {
        return length_;
    }
};

inline LogStream& operator<<(LogStream& s, const Format& format) {
    s.Append(format.Data(), format.Length());
    return s;
}

string FormatSI(int64_t n);

string FormatIEC(int64_t n);

} // namespace dwater

#endif // DWATER_SRC_BASE_LOG_STREAM_H
