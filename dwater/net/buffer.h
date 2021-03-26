// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.25
// Filename:        buffer.h
// Descripton:      应用层的buffer

#ifndef DWATER_NET_BUFFER_H
#define DWATER_NET_BUFFER_H

#include "dwater/base/copyable.h"
#include "dwater/base/string_piece.h"
#include "dwater/base/types.h"
#include "dwater/net/endian.h"

#include <algorithm>
#include <vector>  // buffer
#include <assert.h>
#include <string.h>

namespace dwater {
namespace net {

/// Buffer Model
/// 
/// +------------+-------------------------+-----------------------+
/// | reversed   |   readable(content)     |        writable       |
/// +------------+-------------------------+-----------------------+
/// |            |                         +                       |
/// 0       read_index                  write_index             size
class Buffer : public dwater::copyable {
public:
    static const size_t kcheap_prepend = 8;
    static const size_t kinitial_size = 1024;

    explicit Buffer(size_t initialSize = kinitial_size)
        : buffer_(kcheap_prepend + initialSize),
          reader_index_(kcheap_prepend),
          writer_index_(kcheap_prepend) {
        assert(ReadableBytes() == 0);
        assert(WritableBytes() == initialSize);
        assert(PrependableBytes() == kcheap_prepend);
    }

    void Swap(Buffer& rhs) {
        buffer_.swap(rhs.buffer_);
        std::swap(reader_index_, rhs.reader_index_);
        std::swap(writer_index_, rhs.writer_index_);
    }

    size_t ReadableBytes() const { return writer_index_ - reader_index_; }

    size_t WritableBytes() const { return buffer_.size() - writer_index_; }

    size_t PrependableBytes() const { return reader_index_; }

    const char* Peek() const { return Begin() + reader_index_; }

    const char* FindCRLF() const {
        const char* crlf = std::search(Peek(), BeginWrite(), kCRLF, kCRLF+2);
        return crlf == BeginWrite() ? NULL : crlf;
    }

    const char* FindCRLF(const char* start) const {
        assert(Peek() <= start);
        assert(start <= BeginWrite());
        const char* crlf = std::search(start, BeginWrite(), kCRLF, kCRLF+2);
        return crlf == BeginWrite() ? NULL : crlf;
    }

    const char* FindEOL() const {
        const void* eol = memchr(Peek(), '\n', ReadableBytes());
        return static_cast<const char*>(eol);
    }

    const char* FindEOL(const char* start) const {
        assert(Peek() <= start);
        assert(start <= BeginWrite());
        const void* eol = memchr(start, '\n', BeginWrite() - start);
        return static_cast<const char*>(eol);
    }

    // Retrieve returns void, to prevent
    // string str(Retrieve(ReadableBytes()), ReadableBytes());
    // the evaluation of two functions are unspecified
    void Retrieve(size_t len) {
        assert(len <= ReadableBytes());
        if (len < ReadableBytes()) {
          reader_index_ += len;
        } else {
          RetrieveAll();
        }
    }

    void RetrieveUntil(const char* end) {
        assert(Peek() <= end);
        assert(end <= BeginWrite());
        Retrieve(end - Peek());
    }

    void RetrieveInt64() {
        Retrieve(sizeof(int64_t));
    }

    void RetrieveInt32() {
        Retrieve(sizeof(int32_t));
    }

    void RetrieveInt16() {
        Retrieve(sizeof(int16_t));
    }

    void RetrieveInt8() {
        Retrieve(sizeof(int8_t));
    }

    void RetrieveAll() {
        reader_index_ = kcheap_prepend;
        writer_index_ = kcheap_prepend;
    }

    string RetrieveAllAsString() {
        return RetrieveAsString(ReadableBytes());
    }

    string RetrieveAsString(size_t len) {
        assert(len <= ReadableBytes());
        string result(Peek(), len);
        Retrieve(len);
        return result;
    }

    StringPiece toStringPiece() const {
        return StringPiece(Peek(), static_cast<int>(ReadableBytes()));
    }

    void Append(const StringPiece& str) {
        Append(str.Data(), str.Size());
    }

    void Append(const char* /*restrict*/ data, size_t len) {
        EnsureWritableBytes(len);
        std::copy(data, data+len, BeginWrite());
        hasWritten(len);
    }

    void Append(const void* /*restrict*/ data, size_t len) {
        Append(static_cast<const char*>(data), len);
    }

    void EnsureWritableBytes(size_t len)  {
        if (WritableBytes() < len) {
            MakeSpace(len);
        }
        assert(WritableBytes() >= len);
    }

    char* BeginWrite() { return Begin() + writer_index_; }

    const char* BeginWrite() const { return Begin() + writer_index_; }

    void hasWritten(size_t len) {
        assert(len <= WritableBytes());
        writer_index_ += len;
    }

    void unwrite(size_t len) {
        assert(len <= ReadableBytes());
        writer_index_ -= len;
    }

    ///
    /// Append int64_t using network endian
    ///
    void AppendInt64(int64_t x) {
        int64_t be64 = socket::HostToNetwork64(x);
        Append(&be64, sizeof be64);
    }

    ///
    /// Append int32_t using network endian
    ///
    void AppendInt32(int32_t x) {
        int32_t be32 = socket::HostToNetwork32(x);
        Append(&be32, sizeof be32);
    }

    void AppendInt16(int16_t x) {
        int16_t be16 = socket::HostToNetwork16(x);
        Append(&be16, sizeof be16);
    }

    void AppendInt8(int8_t x) {
        Append(&x, sizeof x);
    }

    ///
    /// Read int64_t from network endian
    ///
    /// Require: buf->ReadableBytes() >= sizeof(int32_t)
    int64_t ReadInt64() {
        int64_t result = PeekInt64();
        RetrieveInt64();
        return result;
    }

    ///
    /// Read int32_t from network endian
    ///
    /// Require: buf->ReadableBytes() >= sizeof(int32_t)
    int32_t ReadInt32() {
        int32_t result = PeekInt32();
        RetrieveInt32();
        return result;
    }

    int16_t ReadInt16() {
        int16_t result = PeekInt16();
        RetrieveInt16();
        return result;
    }

    int8_t ReadInt8() {
        int8_t result = PeekInt8();
        RetrieveInt8();
        return result;
    }

    ///
    /// Peek int64_t from network endian
    ///
    /// Require: buf->ReadableBytes() >= sizeof(int64_t)
    int64_t PeekInt64() const {
        assert(ReadableBytes() >= sizeof(int64_t));
        int64_t be64 = 0;
        ::memcpy(&be64, Peek(), sizeof be64);
        return socket::NetworkToHost64(be64);
    }

    ///
    /// Peek int32_t from network endian
    ///
    /// Require: buf->ReadableBytes() >= sizeof(int32_t)
    int32_t PeekInt32() const {
        assert(ReadableBytes() >= sizeof(int32_t));
        int32_t be32 = 0;
        ::memcpy(&be32, Peek(), sizeof be32);
        return socket::NetworkToHost32(be32);
    }

    int16_t PeekInt16() const {
        assert(ReadableBytes() >= sizeof(int16_t));
        int16_t be16 = 0;
        ::memcpy(&be16, Peek(), sizeof be16);
        return socket::NetworkToHost16(be16);
    }

    int8_t PeekInt8() const {
        assert(ReadableBytes() >= sizeof(int8_t));
        int8_t x = *Peek();
        return x;
    }

    ///
    /// Prepend int64_t using network endian
    ///
    void PrependInt64(int64_t x) {
        int64_t be64 = socket::HostToNetwork64(x);
        Prepend(&be64, sizeof be64);
    }

    ///
    /// Prepend int32_t using network endian
    ///
    void PrependInt32(int32_t x) {
        int32_t be32 = socket::HostToNetwork32(x);
        Prepend(&be32, sizeof be32);
    }

    void PrependInt16(int16_t x) {
        int16_t be16 = socket::HostToNetwork16(x);
        Prepend(&be16, sizeof be16);
    }

    void PrependInt8(int8_t x) {
        Prepend(&x, sizeof x);
    }

    void Prepend(const void* /*restrict*/ data, size_t len) {
        assert(len <= PrependableBytes());
        reader_index_ -= len;
        const char* d = static_cast<const char*>(data);
        std::copy(d, d+len, Begin()+reader_index_);
    }

    void Shrink(size_t reserve) {
        Buffer other;
        other.EnsureWritableBytes(ReadableBytes()+reserve);
        other.Append(toStringPiece());
        Swap(other);
    }

    size_t InternalCapacity() const {
        return buffer_.capacity();
    }

    /// Read data directly into buffer.
    ///
    /// It may implement with readv(2)
    /// @return result of read(2), @c errno is saved
    ssize_t ReadFd(int fd, int* savedErrno);

private:

    char* Begin() { return &*buffer_.begin(); }

    const char* Begin() const { return &*buffer_.begin(); }

    void MakeSpace(size_t len) {
        if (WritableBytes() + PrependableBytes() < len + kcheap_prepend)  {
            buffer_.resize(writer_index_+len);
        } else {
            assert(kcheap_prepend < reader_index_);
            size_t readable = ReadableBytes();
            std::copy(Begin()+reader_index_,
                    Begin()+writer_index_,
                    Begin()+kcheap_prepend);
            reader_index_ = kcheap_prepend;
            writer_index_ = reader_index_ + readable;
            assert(readable == ReadableBytes());
        }
    }

private:
    std::vector<char> buffer_;
    size_t            reader_index_;
    size_t            writer_index_;

    static const char kCRLF[];
};

} // namespace net
} // namespace dwater
#endif // DWATER_NET_BUFFER_H
