// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        logging.cc
// Descripton:      日志最主要的逻辑 

#include "dwater/base/logging.h"
#include "dwater/base/current_thread.h"
#include "dwater/base/timestamp.h"
#include "dwater/base/time_zone.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sstream>

namespace dwater {

__thread char   t_errnobuf[512];
__thread char   t_time[64];
__thread time_t t_last_second;     


const char* strerror_tl(int saved_errno) {
    return strerror_r(saved_errno, t_errnobuf, sizeof t_errnobuf);
}

Logger::LogLevel InitLogLevel() {
    if ( ::getenv("DWATER_LOG_TRACE") ) {
        return Logger::TRACE;
    } else if ( ::getenv("DWATER_LOG_DEBUG") ) {
        return Logger::DEBUG;
    } else {
        return Logger::INFO;
    }
}

Logger::LogLevel g_log_level = InitLogLevel();

const char* LogLevelName[Logger::NUM_LOG_LEVELS] = {
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL "
};
// 在编译期就可以获取字符串长度
class T {
public:
    T(const char* str, unsigned len) : str_(str), len_(len) {
        assert(strlen(str) == len_);
    }
    const char*     str_;
    const unsigned  len_;
};

// 重载LogStream的operator<<
inline LogStream& operator<<(LogStream& s, T v) {
    s.Append(v.str_, v.len_);
    return s;
}

inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v) {
    s.Append(v.data_, v.size_);
    return s;
}

void DefaultOutput(const char* msg, int len) {
    size_t n = fwrite(msg, 1, len, stdout);
    (void)n;
}

void DefaultFlush() {
    fflush(stdout);
}

Logger::OutputFunc g_output = DefaultOutput;
Logger::FlushFunc g_flush = DefaultFlush;
TimeZone g_log_timezone;

} // namespace dwater

using namespace dwater;

Logger::Impl::Impl(LogLevel level, int saved_errno, const SourceFile& file, int line)
    :   time_(Timestamp::Now()),
        stream_(),
        line_(line),
        basename_(file) {
    FormatTime();
    current_thread::Tid();
    stream_ << T(current_thread::TidString(), current_thread::TidStringLength());
    stream_ << T(LogLevelName[level], 6);
    if ( saved_errno != 0 ) {
        stream_ << strerror_tl(saved_errno) << " (errno = " << saved_errno <<") ";
    }
}

void Logger::Impl::FormatTime() {
    int64_t micro_seconds_since_epoch = time_.MicroSecondsSinceEpoch();
    time_t seconds = static_cast<time_t>(micro_seconds_since_epoch / Timestamp::kmicro_seconds_per_second);
    int micro_seconds = static_cast<int>(micro_seconds_since_epoch % Timestamp::kmicro_seconds_per_second);
    if ( seconds != t_last_second ) {
        t_last_second = seconds;
        struct tm tm_time;
        if ( g_log_timezone.valid() ) {
            tm_time = g_log_timezone.ToLocalTime(seconds);
        } else {
            ::gmtime_r(&seconds, &tm_time);
        }
        
        int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
                    tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                    tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        assert(len == 17); (void)len;
    }
    if ( g_log_timezone.valid() ) {
        Format us(".%06dZ ", micro_seconds);
        assert(us.Length() == 8);
        stream_ << T(t_time, 17) << T(us.Data(), 8);
    } else {
        Format us(".%06dZ ", micro_seconds);
        assert(us.Length() == 9);
        stream_ << T(t_time, 17) << T(us.Data(), 9);
    }
}

void Logger::Impl::Finish() {
    stream_ << " - " <<  basename_ << ':' << line_ << '\n';
}

Logger::Logger(SourceFile file, int line) : impl_(INFO, 0, file, line) {}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
    : impl_(level, 0, file, line) {
    impl_.stream_ <<  func <<' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level) 
    :   impl_(level, 0, file, line) {}

Logger::Logger(SourceFile file, int line, bool to_about)
    :   impl_(to_about ? FATAL : ERROR, errno, file, line) {}

Logger::~Logger() {
    impl_.Finish();
    const LogStream::Buffer& buf(Stream().GetBuffer());
    g_output(buf.Data(), buf.Length());
    // 如果FATAL信息，匿名对象Logger析构的时候就要刷新到文件或者标准输出
    if ( impl_.level_ == FATAL ) {
        g_flush();
        abort();
    }
}

void Logger::SetLogLevel(Logger::LogLevel level) {
    g_log_level = level;
}

void Logger::SetOutput(OutputFunc out) {
    g_output = out;
}

void Logger::SetFlush(FlushFunc flush) {
    g_flush = flush;
}

void Logger::SetTimeZone(const TimeZone& tz) {
    g_log_timezone = tz;
}
