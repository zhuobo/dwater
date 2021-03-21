// description: get information of current thread. functions impls in 
// current_thread.cc or thread.cc

#ifndef DWATER_SRC_BASE_CURRENT_THREAD_H
#define DWATER_SRC_BASE_CURRENT_THREAD_H

#include "types.h"

namespace dwater {

namespace current_thread {
// information of threads
extern __thread int         t_cached_tid;  // real thread id
extern __thread char        t_tid_string[32]; // real thread id in string
extern __thread int         t_tid_string_length; // id string len
extern __thread const char* t_thread_name; // thread name 

void CacheTid(); // definition in thread.cc  

inline int Tid() {
    if ( __builtin_expect(t_cached_tid == 0, 0) ) {
        CacheTid();
    }
    return t_cached_tid;
}

inline const char* TidString() {
    return t_tid_string;
}

inline int TidStringLength() {
    return t_tid_string_length;
}

inline const char* Name() {
    return t_thread_name;
}

bool IsMainThread();

void SleepUsec(int64_t user);

// function calling stack frame of current thread
string StackTrace(bool demangle);
} // current_theread

} // namespace dwater

#endif 
