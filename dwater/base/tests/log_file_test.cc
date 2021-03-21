// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.19
// Filename:        log_file_test.cc
// Descripton:      LogFile测试 


#include "../log_file.h"
#include "../logging.h"

#include <unistd.h>

std::unique_ptr<dwater::LogFile> g_log_file;

void OutPutFunc(const char* msg, int len) {
    g_log_file->Append(msg, len);
}

void FlushFunc() {
    g_log_file->Flush();
}

int main(int argc, char* argv[]) {
    char name[256] = { '\0' };
    strncpy(name, argv[0], sizeof name - 1);
    g_log_file.reset(new dwater::LogFile(::basename(name), 200 * 1000));
    dwater::Logger::SetOutput(OutPutFunc);
    dwater::Logger::SetFlush(FlushFunc);

    dwater::string line = "ABCDEFGHIJKLMNOPQRISTUVWXYZ abcdefghijklmnopqristuvwxyz 1234567890";
    for ( int i = 0; i < 10000; ++i ) {
        LOG_INFO << line << i;
        usleep(1000);
    }
}



