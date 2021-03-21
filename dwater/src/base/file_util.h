// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        file_util.h
// Descripton:      最底层的文件类，封装了文件的打开、写入并在析构的时候关闭文件
// Append函数直接写文件


#ifndef DWATER_SRC_BASE_FILE_UTIL_H
#define DWATER_SRC_BASE_FILE_UTIL_H

#include "string_piece.h"
#include "noncopable.h"

#include <sys/types.h> // for type off_t

namespace dwater {

namespace file_util {

// 这里的类都包含了缓冲区用于存储将要写入到文件的数据
class ReadSmallFile : noncopyable {
public:
    static const int kbuffer_size = 64 * 1024;

public:
    ReadSmallFile(StringArg filename);

    ~ReadSmallFile();

    template<typename String>
    int ReadToString(int max_size,
                     String* content,
                     int64_t* file_size,
                     int64_t* modify_time,
                     int64_t* create_time);

    int ReadToBuffer(int* size);

    const char* Buffer() const { return buf_; }

private:
    int     fd_;
    int     err_;
    char    buf_[kbuffer_size];
}; // class ReadSmallFile

template<typename String>
int ReadFile(StringArg filename,
             int max_size,
             String* content,
             int64_t* file_size = NULL,
             int64_t* modify_time = NULL,
             int64_t* create_time = NULL) {
    ReadSmallFile file(filename);
    return file.ReadToString(max_size, content, file_size, modify_time, create_time);
}

class AppendFile : noncopyable {
public:
    explicit AppendFile(StringArg filename);

    ~AppendFile();

    void Append(const char* logline, size_t len);

    void Flush();

    off_t WriteBytes() const { return written_bytes_; }

private:
    size_t Write(const char* logline, size_t len);

private:
    FILE*   fp_;
    char    buffer_[64 * 1024];
    off_t   written_bytes_;

}; // class AppendFile

} // namespace file_util

} // namespace dwater

#endif // DWATER_SRC_BASE_FILE_UTIL_H
