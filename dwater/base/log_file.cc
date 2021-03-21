// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.19
// Filename:        log_file.cc
// Descripton:      log_file.h的实现，用于实现日志滚动

#include "log_file.h"

#include "file_util.h"
#include "process_info.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>

using namespace dwater;

LogFile::LogFile(const string& basename,
                 off_t roll_size,
                 bool thread_safe,
                 int flush_interval,
                 int check_every_N)
    : basename_(basename),
      roll_size_(roll_size),
      flush_interval_(flush_interval),
      check_every_n_(check_every_N),
      count_(0),
      mutex_(thread_safe ? new MutexLock : NULL),
      start_of_period_(0),
      last_roll_(0),
      last_flush_(0) {
    assert(basename.find('/') == string::npos);
    RollFile();
}

LogFile::~LogFile() = default;

void LogFile::Append(const char* logline, int len) {
    if ( mutex_ ) {
        MutexLockGuard lock(*mutex_);
        AppendUnlocked(logline, len);
    } else {
        AppendUnlocked(logline, len);
    }
}

void LogFile::Flush() {
    if ( mutex_ ) {
        MutexLockGuard lock(*mutex_);
        file_->Flush();
    } else {
        file_->Flush();
    }
}

// 
void LogFile::AppendUnlocked(const char* logline, int len) {
    file_->Append(logline, len); //进行IO
    // 超过日志滚动大小,滚动文件
    if ( file_->WriteBytes() > roll_size_ ) {
        RollFile();
    } else {
        ++count_; // 记录次数,写入文件的次数到了也滚动
        if ( count_ >= check_every_n_ ) {
            count_ = 0;
            time_t now = ::time(NULL);
            time_t this_period = now / kroll_per_seconds_ * kroll_per_seconds_;
            if ( this_period != start_of_period_ ) {
                RollFile();
            } else if ( now - last_flush_ > flush_interval_ ) {
                last_flush_ = now;
                file_->Flush();
            }
        }
    }
}

// 滚动日志
bool LogFile::RollFile() {
    time_t now = 0;
    string filename = GetLogFileName(basename_, &now);
    time_t start = now / kroll_per_seconds_ * kroll_per_seconds_;

    if (  now > last_roll_  ) {
        last_roll_ = now;
        last_flush_ = now;
        start_of_period_ = start;
        file_.reset(new file_util::AppendFile(filename));
        return true;
    }
    return false;
}

string LogFile::GetLogFileName(const string& basename, time_t* now) {
    string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;

    char timebuf[32];
    struct tm tm;
    *now = time(NULL);

    gmtime_r(now, &tm);
    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);

    filename += timebuf;

    filename += process_info::Hostname();

    char pidbuf[32];
    snprintf(pidbuf, sizeof pidbuf, ".%d", process_info::Pid());
    filename += pidbuf;

    filename += ".log";

    return filename;
}

