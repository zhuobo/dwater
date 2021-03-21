// Author: Drinkwater
// Email: tanzhuobo@gmail.com
// Last modified: 2021.03.16
// Filename: noncopable.h
// Descripton: 继承这个类的派生类都无法被调用拷贝构造函数或者operation=

#ifndef DWATER_SRC_BASE_NONCOPYABLE_H
#define DWATER_SRC_BASE_NONCOPYABLE_H

namespace dwater {

class noncopyable {
public:
    noncopyable(const noncopyable&) = delete;
    
    void operator=(const noncopyable&) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

} // namespace dwater

#endif // DWATER_SRC_BASE_NONCOPYABLE_H
