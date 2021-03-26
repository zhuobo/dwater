// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        logging.h
// Descripton:      最主要的日志逻辑 

#ifndef DWATER_SRC_BASE_LOGGING_H
#define DWATER_SRC_BASE_LOGGING_H

#include "dwater/base/log_stream.h"
#include "dwater/base/timestamp.h"

namespace dwater {

// forward declaration
class TimeZone;

class Logger {
public:
    enum LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS,
    };

    class SourceFile {
    public:
        template<int N>
        SourceFile(const char (&arr)[N]) : data_(arr), size_(N - 1) {
            const char* slash = strchr(data_, '/');
            if ( slash ) {
                data_ = slash + 1;
                size_ -= static_cast<int>(data_ - arr);
            }
        }

        explicit SourceFile(const char* filename) : data_(filename) {
            const char* slash = strchr(filename, '/');
            if ( slash ) {
                data_ = slash + 1;
            } else {
                size_ = static_cast<int>(strlen(data_));
            }
        }

    // members
        const char* data_;
        int         size_;
    };

    Logger(SourceFile file, int line);
    Logger(SourceFile file, int line, LogLevel level);
    Logger(SourceFile file, int line, LogLevel level, const char* func);
    Logger(SourceFile file, int line, bool to_abort);
    ~Logger();

    LogStream& Stream() { return impl_.stream_; }

    static LogLevel logLevel();
    static void SetLogLevel(LogLevel level);

    typedef void (*OutputFunc)(const char* msg, int len);
    typedef void (*FlushFunc)();
    static void SetOutput(OutputFunc);
    static void SetFlush(FlushFunc);
    static void SetTimeZone(const TimeZone& tz);

private:
    class Impl {
    public:
        typedef Logger::LogLevel LogLevel;
        Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
        void FormatTime();
        void Finish();

        // members
        Timestamp   time_;
        LogStream   stream_;
        LogLevel    level_;
        int         line_;
        SourceFile  basename_;
    };

    Impl impl_;
};

extern Logger::LogLevel g_log_level;

inline Logger::LogLevel Logger::logLevel() {
    return g_log_level;
}

// 宏，生成Logger的临时对象，讲数据存储在Logger对象的缓存区中
#define LOG_TRACE if (dwater::Logger::logLevel() <= dwater::Logger::TRACE) \
  dwater::Logger(__FILE__, __LINE__, dwater::Logger::TRACE, __func__).Stream()
#define LOG_DEBUG if (dwater::Logger::logLevel() <= dwater::Logger::DEBUG) \
  dwater::Logger(__FILE__, __LINE__, dwater::Logger::DEBUG, __func__).Stream()
#define LOG_INFO if (dwater::Logger::logLevel() <= dwater::Logger::INFO) \
  dwater::Logger(__FILE__, __LINE__).Stream()
#define LOG_WARN dwater::Logger(__FILE__, __LINE__, dwater::Logger::WARN).Stream()
#define LOG_ERROR dwater::Logger(__FILE__, __LINE__, dwater::Logger::ERROR).Stream()
#define LOG_FATAL dwater::Logger(__FILE__, __LINE__, dwater::Logger::FATAL).Stream()
#define LOG_SYSERR dwater::Logger(__FILE__, __LINE__, false).Stream()
#define LOG_SYSFATAL dwater::Logger(__FILE__, __LINE__, true).Stream()

const char* strerror_tl(int savedErrno);

// Taken from glog/logging.h
//
// Check that the input is non NULL.  This very useful in constructor
// initializer lists.

#define CHECK_NOTNULL(val) \
  ::dwater::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

// A small helper for CHECK_NOTNULL().
template <typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char *names, T* ptr)
{
  if (ptr == NULL)
  {
   Logger(file, line, Logger::FATAL).Stream() << names;
  }
  return ptr;
}

} // namespace dwater

#endif // DWATER_SRC_BASE_LOGGING_H
