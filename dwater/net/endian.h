// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.22
// Filename:        endian.h
// Descripton:      网络主机数据格式的转换 


#ifndef DWATER_NET_ENDIAN_H
#define DWATER_NET_ENDIAN_H

#include <stdint.h>
#include <endian.h>

namespace dwater {

namespace net {

namespace  socket {


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"

inline uint64_t HostToNetwork64(uint64_t host64) {
    return htobe64(host64);
}

inline uint32_t HostToNetwork32(uint32_t host32) {
    return htobe32(host32);
}

inline uint16_t HostToNetwork16(uint16_t host16) {
    return htobe16(host16);
}

inline uint64_t NetworkToHost64(uint64_t net64) {
    return be64toh(net64);
}

inline uint32_t NetworkToHost32(uint32_t net32) {
    return be32toh(net32);
}

inline uint16_t NetworkToHost16(uint16_t net16) {
    return be16toh(net16);
}

#pragma GCC diagnostic pop
} // namespace socket
} // namespace net
} // namspace dwater

#endif // DWATER_NET_ENDIAN_H
