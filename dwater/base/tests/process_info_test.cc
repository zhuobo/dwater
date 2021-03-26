// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.19
// Filename:        process_info_test.cc
// Descripton:      process_info命名空间中关于进程线程相关信息函数的操作


#include "../process_info.h"

#include <stdio.h>

#define __STDC_FORMAT_MACROS // for inttypes.h
#include <inttypes.h>

using namespace dwater;
using namespace dwater::process_info;

int main() {
    printf("pid = %d\n", Pid());
    printf("uid = %d\n", Uid());
    printf("euid = %d\n", Euid());
    printf("start time = %s\n", StartTime().ToFormattedString().c_str());
    printf("hostname = %s\n", Hostname().c_str());
    printf("opened files = %d\n", OpenedFiles());
    printf("threads = %zd\n", Threads().size());
    printf("num threads = %d\n", NumThreads());
    printf("status = %s\n", ProcStatus().c_str());
}



