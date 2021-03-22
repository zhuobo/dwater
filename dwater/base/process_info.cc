// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.19
// Filename:        process_info.cc 
// Descripton:      命名空间process_info中一些获取进程信息的函数的实现，其中很多
// 都是系统函数的wrapper

#include "dwater/base/process_info.h"
#include "dwater/base/current_thread.h"
#include "dwater/base/file_util.h"

#include <assert.h>
#include <dirent.h>
#include <pwd.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/times.h>

#include <algorithm>

namespace dwater {

namespace detail {

__thread int t_num_opened_files = 0;

// 打开的文件描述符过滤，在/proc/self/fd中有打开的文件描述符的符号链接，
// 这个函数可以用来统计打开了多少个文件
int FdDirFilter(const struct dirent* d) {
    if ( ::isdigit(d->d_name[0]) ) {
        ++t_num_opened_files;
    }
    return 0;
}

// 打开的文件集合
__thread std::vector<pid_t>* t_pids = NULL;

int TaskDirFilter(const struct dirent* d) {
    if ( ::isdigit(d->d_name[0]) ) {
        t_pids->push_back(atoi(d->d_name));
    }
    return 0;
}

int ScanDir(const char* dir_path, int (*filter)(const struct dirent*)) {
    struct dirent** namelist  = NULL;
    int result = ::scandir(dir_path, &namelist, filter, alphasort);
    assert(namelist == NULL);
    return result;
}

Timestamp g_start_time = Timestamp::Now();

int g_clock_ticks = static_cast<int>(::sysconf(_SC_CLK_TCK));
int g_page_size = static_cast<int>(::sysconf(_SC_PAGE_SIZE));
} // namespace detail

} // namespace dwater

using namespace dwater;
using namespace dwater::detail;

pid_t process_info::Pid() {
    return ::getpid();
}

string process_info::PidString() {
    char buf[32];
    snprintf(buf, sizeof buf, "%d", Pid());
    return buf;
}

uid_t process_info::Uid() {
    return ::getuid();
}

string process_info::Username() {
    struct passwd pwd;
    struct passwd* result = NULL;
    char buf[8192];
    const char* name = "unknownuser";

    getpwuid_r(Uid(), &pwd, buf, sizeof buf, &result);
    if ( result ) {
        name = pwd.pw_name;
    }
    return name;
}

uid_t process_info::Euid() {
    return ::geteuid();
}

Timestamp process_info::StartTime() {
    return g_start_time;
}

int process_info::ClockTicksPerSecond() {
    return g_clock_ticks;
}

int process_info::PageSize() {
    return g_page_size;
}

bool process_info::IsDebugBuild() {
#ifdef NDEBUG
    return false;
#else
    return true;
#endif
}

string process_info::Hostname() {
    char buf[256];
    if ( ::gethostname(buf, sizeof buf) == 0 ) {
        buf[sizeof(buf) - 1] = '\0';
        return buf;
    } else {
        return  "unknownhost";
    }
}

string process_info::Procname() {
    return Procname(ProcStat()).AsString();
}

// 获取进程名称
StringPiece process_info::Procname(const string& stat) {
    StringPiece  name;
    size_t lp = stat.find('(');
    size_t rp = stat.find(')');
    if ( lp != string::npos && rp != string::npos && lp < rp ) {
        name.Set(stat.data() + lp + 1, static_cast<int>(rp - lp - 1));
    }
    return name;
}


string process_info::ProcStatus() {
    string result;
    file_util::ReadFile("/proc/self/status", 65536, &result);
    return result;
}

string process_info::ProcStat() {
    string result;
    file_util::ReadFile("/proc/self/stat", 65536, &result);
    return result;
}

// 线程状态
string process_info::ThreadStat() {
    char buf[64];
    snprintf(buf, sizeof buf, "/proc/self/task/%d/stat", current_thread::Tid());
    string result;
    file_util::ReadFile(buf, 65536, &result);
    return result;
}

// 获取当前进程的绝对路径
string process_info::ExePath() {
    string result;
    char buf[1024];
    ssize_t n = ::readlink("/proc/self/exe", buf, sizeof buf);
    if ( n > 0 ) {
        result.assign(buf, n);
    } 
    return result;
}

// 打开文件描述符的数量
int process_info::OpenedFiles() {
    t_num_opened_files = 0;
    ScanDir("/proc/self/fd", FdDirFilter);
    return t_num_opened_files;
}

// 最大可以打开的文件描述符
int process_info::MaxOpenFiles() {
    struct rlimit rl;
    if ( ::getrlimit(RLIMIT_NOFILE, &rl) ) {
        return OpenedFiles();
    } else {
        return static_cast<int>(rl.rlim_cur);
    }
}

// 获取cpu时间
process_info::CPUTime process_info::CpuTime() {
    process_info::CPUTime t;
    struct tms tms;
    if ( ::times(&tms) >= 0 ) {
        const double hz = static_cast<double>(ClockTicksPerSecond());
        t.user_seconds = static_cast<double>(tms.tms_utime) / hz;
        t.system_seconds = static_cast<double>(tms.tms_stime) / hz;
    }
    return t;
}

// 获取当前进程中线程的数量
//线程数目，通过命令`cat /proc/进程id/status | grep 'Threads' 可以获得线程数目`
int process_info::NumThreads() {
    int result = 0;
    string status = ProcStatus();
    size_t pos = status.find("Threads:");
    if ( pos != string::npos ) {
        result = ::atoi(status.c_str() + pos + 8);
    }
    return result;
}

// 进程编号的集合，升序排序 
// 通过命令'ls /proc/进程id/task 可以获得进程中的线程号'
std::vector<pid_t> process_info::Threads() {
    std::vector<pid_t> result;
    t_pids = &result;
    ScanDir("/proc/self/task", TaskDirFilter);
    t_pids = NULL;
    std::sort(result.begin(), result.end());
    return result;
}
