// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        file_util.cc
// Descripton:      file_util.h中两个类的实现 

#include "dwater/base/file_util.h"
#include "dwater/base/logging.h"

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>


using namespace dwater;

// 构造函数，打开一个文件，设置文件缓冲区为当前对象中的缓冲区
file_util::AppendFile::AppendFile(StringArg filename)
    :   fp_(::fopen(filename.Cstr(), "ae")), // 当 执行execxx文件会被关闭
        written_bytes_(0) {
    assert(fp_);
    ::setbuffer(fp_, buffer_, sizeof buffer_);
}

// 文件析构函数
file_util::AppendFile::~AppendFile() {
    ::fclose(fp_);
}

void file_util::AppendFile::Append(const char* logline, const size_t len) {
    size_t n = Write(logline, len);
    size_t remain = len - n;
    while ( remain > 0 ) {
        size_t x = Write(logline + n, remain);
        if ( x == 0 ) {
            int err = ferror(fp_);
            if ( err ) {
                fprintf(stderr, "AppendFile::Append() failed %s\n", dwater::strerror_tl(err));
            }
            break;
        }
        n += x;
        remain = len - n;
    }
    written_bytes_ += len;
}

void file_util::AppendFile::Flush() {
    ::fflush(fp_);
}

// Append中调用这个函数，使用无锁的fwrite函数，将数据写入到文件中，使用无锁是因为
// Append的调用是被加锁的，那么这里的执行肯定就被加锁，而C语言的IO函数都是线程安全
// 的，内部都会加锁，因此为了效率更高，我们就不要调用线程安全的版本了，因为这里的执行
// 被加锁，可以做到只有一个线程同时访问
size_t file_util::AppendFile::Write(const char* logline, size_t len) {
    return  ::fwrite_unlocked(logline, 1, len, fp_);
}

file_util::ReadSmallFile::ReadSmallFile(StringArg filename)
    :   fd_(::open(filename.Cstr(), O_RDONLY | O_CLOEXEC)),
        err_(0) {
    buf_[0] = '\0';
    if ( fd_ < 0 ) {
        err_ = errno;
    }
}

file_util::ReadSmallFile::~ReadSmallFile() {
    if ( fd_ >= 0 ) {
        ::close(fd_);
    }
}


template<typename String>
int file_util::ReadSmallFile::ReadToString(int max_size,
                                           String* content, 
                                           int64_t* file_size,
                                           int64_t* modify_time,
                                           int64_t* create_time) {
    static_assert(sizeof(off_t) == 8, "_FILE_OFFSET_BITS = 64");
    assert(content != NULL);
    int err = err_;
    if ( fd_ > 0 ) {
        content->clear();
        if ( file_size ) {
            struct stat statbuf;
            if ( ::fstat(fd_, &statbuf) ==0 ) {
                if ( S_ISREG(statbuf.st_mode) == 0 ) {
                    *file_size = statbuf.st_size;
                    content->reserve(static_cast<int>(std::min(implicit_cast<int64_t>(max_size), *file_size)));
                } else if ( S_ISDIR(statbuf.st_mode) ) {
                    err = EISDIR;
                } 
                if ( modify_time ) {
                    *modify_time = statbuf.st_mtime;
                }
                if ( create_time ) {
                    *create_time = statbuf.st_ctime;
                }
            } else {
                err = errno;
            }
        }

        while ( content->size() < implicit_cast<size_t>(max_size) ) {
            size_t to_read = std::min(implicit_cast<size_t>(max_size) - content->size(), sizeof(buf_));
            ssize_t n = ::read(fd_, buf_, to_read);
            if ( n > 0 ) {
                content->append(buf_, n);
            } else {
                if ( n < 0 ) {
                    err = errno;
                }
                break;
            }
        }
    }

    return err;
}

int file_util::ReadSmallFile::ReadToBuffer(int* size) {
    int err = err_;
    if ( fd_ >= 0 ) {
        ssize_t n = ::pread(fd_, buf_, sizeof(buf_) - 1, 0);
        if ( n >= 0 ) {
            if ( size ) {
                *size = static_cast<int>(n);
            }
            buf_[n] = '\0';
        } else {
            err = errno;
        }
    }
    return err;
}

template int file_util::ReadFile(StringArg filename,
                                int maxSize,
                                string* content,
                                int64_t*, int64_t*, int64_t*);

template int file_util::ReadSmallFile::ReadToString(
    int maxSize,
    string* content,
    int64_t*, int64_t*, int64_t*);

