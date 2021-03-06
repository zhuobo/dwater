// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.25
// Filename:        weak_callback.h
// Descripton:       

#ifndef DWATER_BASE_WEAK_CALLBACK_H
#define DWATER_BASE_WEAK_CALLBACK_H
#include <functional>
#include <memory>

namespace dwater {
template<typename CLASS, typename... ARGS>

///
/// 弱回调：如果对象还活着，就尝试调用它的成员函数，否则就不必调用
///
/// 利用weak_ptr，在回调的时候 尝试提升为shared_ptr，如果提升成功，说明回调的对象
/// 还活着，那么就可以执行回调，否则就不必执行。
///
class WeakCallback
{
 public:

  WeakCallback(const std::weak_ptr<CLASS>& object,
               const std::function<void (CLASS*, ARGS...)>& function)
    : object_(object), function_(function)
  {
  }

  // Default dtor, copy ctor and assignment are okay

  void operator()(ARGS&&... args) const
  {
    std::shared_ptr<CLASS> ptr(object_.lock());
    if (ptr)
    {
      function_(ptr.get(), std::forward<ARGS>(args)...);
    }
  }

 private:

  std::weak_ptr<CLASS> object_;
  std::function<void (CLASS*, ARGS...)> function_;
};

template<typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> MakeWeakCallback(const std::shared_ptr<CLASS>& object,
                                              void (CLASS::*function)(ARGS...))
{
  return WeakCallback<CLASS, ARGS...>(object, function);
}

template<typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> MakeWeakCallback(const std::shared_ptr<CLASS>& object,
                                              void (CLASS::*function)(ARGS...) const)
{
  return WeakCallback<CLASS, ARGS...>(object, function);
}

} // dwater
#endif // DWATER_BASE_WEAK_CALLBACK_H
