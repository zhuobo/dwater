// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        log_file.h
// Descripton:      为了应对日志文件过大的问题实现的文件滚动 


#ifndef DWATER_SRC_BASE_LOG_FILE_H
#define DWATER_SRC_BASE_LOG_FILE_H

#include "dwater/base/mutex.h"
#include "dwater/base/types.h"

#include <memory>

namespace dwater {

namespace file_util {
class AppendFile;
} // namespace file_util

class LogFile : noncopyable {
public:
    LogFile(const string& basename,
            off_t roll_size,
            bool thread_safe = true,
            int flush_interval = 3,
            int check_every_n = 1024);
    ~LogFile();

    void Append(const char* logline, int len);

    void Flush();

    bool RollFile();

private:
    void AppendUnlocked(const char* logline, int len);

    static string GetLogFileName(const string& basename, time_t* now);

private:
    const string basename_; // 日志文件名
    const off_t roll_size_; // 日志文件大小达到roll_size_,就生成一个新的文件
    const int flush_interval_; // 写入到文件的间隔时间
    const int check_every_n_; // 写这么多次就滚动一次日志

    int count_; // 日志写入文件的次数

    std::unique_ptr<MutexLock> mutex_; // 为写日志加锁
    time_t start_of_period_; // 开始记录日志的时间
    time_t last_roll_;  // 上次滚动日志时间
    time_t last_flush_; // 上次写到文件的时间
    std::unique_ptr<file_util::AppendFile> file_; // 中间有一个buffer

    const static int kroll_per_seconds_ = 60*60*24; // 一天滚动一次日志
}; // class LogFile

} // namesapce dwater

#endif  // DWATER_SRC_BASE_LOG_FILE_H

