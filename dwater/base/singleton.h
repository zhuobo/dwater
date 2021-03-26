// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.20
// Filename:        singletonh
// Descripton:      一个线程安全的singleton

#ifndef DWATER_SRC_BASE_SINGLETON_H
#define DWATER_SRC_BASE_SINGLETON_H

#include "dwater/base/noncopable.h"

#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

namespace dwater {

namespace detail {

template<typename T>
struct has_no_destroy {
    template <typename C> static char test(decltype(&C::no_destroy));
    template <typename C> static int32_t test(...);
    const static bool value = sizeof(test<T>(0)) == 1;
};

} // namespace detail

template<typename T>
class Singleton : noncopyable {
public:
    Singleton() = delete;
    ~Singleton() = delete;

    // 获取对象，由pthread_once保证Init在多个线程只被执行一次
    static T& Instance() {
        pthread_once(&ponce_, &Singleton::Init);
        assert(value_ != NULL);
        return *value_;
    }

private:
    // 对象的销毁
    static void Destroy() {
        // 如果对象是一个不完全类型，编译器就会报错
        // 不完全类型只有声明没有定义，不能用来创建对象
        typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
        T_must_be_complete_type dummy;
        (void) dummy;

        delete value_;
        value_ = NULL;
    }

        
    // Init调用构造函数
    static void Init() {
        value_ = new T();
        // 当参数是类，并且 没有has_no_destory方法时，注册一个atexit函数
        // 销毁创建的对象
        if ( !detail::has_no_destroy<T>::value ) {
            ::atexit(Destroy); 
        }
    }

private:
    static pthread_once_t ponce_;
    static T*             value_;
};

// 两个静态成员变量在类的外部初始化
template<typename T>
pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

template<typename T>
T* Singleton<T>::value_ = NULL;

} // namespace dwater

#endif // DWATER_SRC_BASE_SINGLETON_H



