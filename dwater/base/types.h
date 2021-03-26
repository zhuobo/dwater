#ifndef DWATER_SRC_BASE_THPES_H
#define DWATER_SRC_BASE_THPES_H

#include <stdint.h>
#include <string.h>
#include <string>

#ifndef NDEBUG
#include <assert.h>
#endif

namespace dwater {

// dwater::string is also std::string, they are the same
// but I would like to use stringpiece
using std::string;

inline void MemZero(void *p, size_t n) {
    memset(p, 0, n);
}

template<typename To, typename From>
inline To implicit_cast(From const &from) {
    return from;
}

template<typename To, typename From>
inline To down_cast(From *from) {
    if ( false ) {
        implicit_cast<From*, To>(0);
    }

    
#if !defined(NDEBUG) && !defined(GOOGLE_PROTOBUF_NO_RTTI)
    assert(from == NULL || dynamic_cast<To>(from) != NULL);
#endif

    return static_cast<To>(from);
}
} // namespace dwater

#endif
