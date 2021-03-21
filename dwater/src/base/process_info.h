// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        process_info.h
// Descripton:      在命名空间process_info中定义了一些获取进程信息的函数 

#ifndef DWATER_SRC_BASE_PROCESS_INFO_H
#define DWATER_SRC_BASE_PROCESS_INFO_H

#include "string_piece.h"
#include "types.h"
#include "timestamp.h"

#include <vector>
#include <sys/types.h>

namespace dwater {

namespace process_info {

pid_t Pid();
string PidString();
uid_t Uid();
string Username();
uid_t Euid();
Timestamp StartTime();
int ClockTicksPerSecond();
int PageSize();
bool IsDebugBuild();  // constexpr

string Hostname();
string Procname();
StringPiece Procname(const string& stat);

/// read /proc/self/status
string ProcStatus();

/// read /proc/self/stat
string ProcStat();

/// read /proc/self/task/tid/stat
string ThreadStat();

/// readlink /proc/self/exe
string ExePath();

int OpenedFiles();
int MaxOpenFiles();

struct CPUTime {
    double user_seconds;
    double system_seconds;

    CPUTime() :user_seconds (0.0), system_seconds(0.0) { }

    double Total() const { return user_seconds + system_seconds; }
};


CPUTime CpuTime();

int NumThreads();

std::vector<pid_t> Threads();

} // namespace process_info

} // namespace dwater 


#endif  // DWATER_SRC_BASE_PROCESS_INFO_H
