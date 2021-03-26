// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        copyable.h
// Descripton:      这个类的派生类是是可以调用拷贝构造函数以及operator=的，但是
// 必须是值类型（value type），而不是reference type，才可以被复制


#ifndef DWATER_SRC_BASE_COPYABLE_H
#define DWATER_SRC_BASE_COPYABLE_H

namespace dwater {

class copyable {
protected:
    copyable() = default;
    ~copyable() = default;
};

} // namespace dwater


#endif // DWATER_SRC_BASE_COPYABLE_H



