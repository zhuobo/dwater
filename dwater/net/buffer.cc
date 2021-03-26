// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.25
// Filename:        buffer.cc
// Descripton:      

#include "dwater/net/buffer.h"
#include "dwater/net/socket_ops.h"
#include <errno.h>
#include <sys/uio.h> // for struct iovec

using namespace dwater;
using namespace dwater::net;

const char Buffer::kCRLF[] = "\r\n";

const size_t Buffer::kcheap_prepend;
const size_t Buffer::kinitial_size;

ssize_t Buffer::ReadFd(int fd, int* saved_errno) {
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = WritableBytes();
    vec[0].iov_base = Begin() + writer_index_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const int iov_count = (writable < sizeof(extrabuf)) ? 2 : 1;
    const ssize_t n = socket::Readv(fd, vec, iov_count);
    if ( n < 0 ) {
        *saved_errno = errno;
    } else if ( implicit_cast<size_t>(n) <= writable ) {
        writer_index_ += n;
    } else {
        writer_index_ = buffer_.size();
        Append(extrabuf, n - writable);
    }
    return n;
}
