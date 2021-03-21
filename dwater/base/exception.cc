// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.
// Filename:        exception.cc
// Descripton:      Excepion类的实现

#include "exception.h"
#include "current_thread.h"

namespace dwater {

Exception::Exception(string msg)
    : message_(std::move(msg)),
      stack_(current_thread::StackTrace(false)) {  }

} // namespace dwater



