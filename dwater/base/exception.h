// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        exception.h
// Descripton:      异常处理类，继承自std::exception，提供打印线程调用栈的功能

#ifndef DWATER_SRC_BASE_EXCEPTION_H
#define DWATER_SRC_BASE_EXCEPTION_H

#include "dwater/base/types.h"

#include <exception>

namespace dwater {
    
class Exception : public std::exception {
private:
    string message_;
    string stack_;

public:
    Exception(string what);
    ~Exception() noexcept override = default;

    const char* what() const noexcept override {
        return message_.c_str();
    }

    const char* StackTrace() const noexcept {
        return stack_.c_str();
    }
};

} // namespace dwater


#endif // DWATER_SRC_BASE_EXCEPTION_H


