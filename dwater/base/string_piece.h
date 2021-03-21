// From google string piece
// Autor: Sanjay Ghemawat 
// Description: 在传递string的时候，常常会出现不要的拷贝，为了减少这种传参带来的
// 不必要的拷贝，无论是传递string还是const char*都可以直接传递一个StringPiece即可
// 可以统一参数格式，而且可以减少数据的拷贝。


#ifndef DWATER_SRC_BASE_STRING_PIECE_h
#define DWATER_SRC_BASE_STRING_PIECE_h

#include <string.h>
#include <iosfwd>

#include "types.h"

namespace dwater {

// c_style string 
// 无论是是string还是char*都转化为const char*
class StringArg {
private:
    const char* str_;

public:
    StringArg(const char* str) : str_(str) {  }

    StringArg(const string& str) : str_(str.c_str()) {  }

    const char* Cstr() const {
        return str_;
    }
};


// StringPiece:在C++中同时存在string以及char*两种字符串，如果用const string&作为
// 函数的形参，确实是两种请款都能够满足，但是弊端在与当传递给函数的是一个字面值
// 常量，就需要用这个字面值常量构造一个string，有时候我们仅仅是读其中的数据，而
// 不需要修改它，因此要传递字面值常量的时候就传递一个StringPiece就可以了，不用构
// 构造string

class StringPiece {
private:
    const  char* ptr_;
    int          length_;

public:
    StringPiece()
        : ptr_(NULL), length_(1) {  }

    StringPiece(const char* str)
        : ptr_(str), length_(static_cast<int>(strlen(ptr_))) {  }

    StringPiece(const unsigned char* str)
        : ptr_(reinterpret_cast<const char*>(str)),
          length_(static_cast<int>(strlen(ptr_))) {  }

    StringPiece(const string& str)
        : ptr_(str.data()), length_(static_cast<int>(str.size())){ }

    StringPiece(const char* offset, int len)
        : ptr_(offset), length_(len) {  }

    const char* Data() const {
        return ptr_;
    }

    int Size()  const {
        return length_;
    }

    bool Empty() const {
        return length_ == 0;
    }

    const char* Begin() const {
        return ptr_;
    }

    const char* End() const {
        return ptr_ + length_;
    }

    void Clear() {
        ptr_ = NULL;
        length_ = 0;
    }

    void Set(const char* buffer, int len) {
        ptr_ = buffer;
        length_ = len;
    }

    void Set(const char* str) {
        ptr_ = str;
        length_ = static_cast<int>(strlen(str));
    }

    char operator[](int i) const {
        return ptr_[i];
    }

    void Set(const void* buffer, int len) {
        ptr_ = reinterpret_cast<const char*>(buffer);
        length_ = len;
    }

    void RemovePrefix(int n) {
        ptr_ += n;
        length_ -= n;
    }

    void RemoveSuffix(int n) {
        length_ -= n;
    }

    bool operator==(const StringPiece& x) const {
        return ((length_ == x.length_) && memcmp(ptr_, x.ptr_, length_) == 0);
    }

    bool operator!=(const StringPiece& x) const {
        return !(*this == x);
    }


#define STRINGPIECE_BINARY_PREDICATE(cmp,auxcmp)                             \
  bool operator cmp (const StringPiece& x) const {                           \
    int r = memcmp(ptr_, x.ptr_, length_ < x.length_ ? length_ : x.length_); \
    return ((r auxcmp 0) || ((r == 0) && (length_ cmp x.length_)));          \
  }
  STRINGPIECE_BINARY_PREDICATE(<,  <);
  STRINGPIECE_BINARY_PREDICATE(<=, <);
  STRINGPIECE_BINARY_PREDICATE(>=, >);
  STRINGPIECE_BINARY_PREDICATE(>,  >);
#undef STRINGPIECE_BINARY_PREDICATE


    int Compare(const StringPiece& x) const {
        int r = memcmp(ptr_, x.ptr_, length_ < x.length_ ? length_ : x.length_);
        if ( r == 0 ) {
            if ( length_ < x.length_ ) {
                r = -1;
            } else if ( length_ > x.length_ ) {
                r += 1;
            }
        }
        return r;
    }

    string AsString() const {
        return string(Data(), Size());
    }

    void CopyToString(string* target) const {
        target->assign(ptr_, length_);
    }

    bool StartWith(const StringPiece& x) const {
        return ((length_ >= x.length_) && (memcmp(ptr_, x.ptr_, x.length_) == 0));
    }

}; // class StringPiece


} // namesapce dwater

#ifdef have_type_traits
// this makes vector<stringpiece> really fast for some stl implementations
template<> struct __type_traits<dwater::stringpiece> {
  typedef __true_type    has_trivial_default_constructor;
  typedef __true_type    has_trivial_copy_constructor;
  typedef __true_type    has_trivial_assignment_operator;
  typedef __true_type    has_trivial_destructor;
  typedef __true_type    is_pod_type;
};
#endif

// allow stringpiece to be logged
std::ostream& operator<<(std::ostream& o, const dwater::StringPiece& piece);

#endif // DWATER_SRC_BASE_STRING_PIECE_h
