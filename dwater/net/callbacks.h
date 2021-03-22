// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.22
// Filename:        callbacks.h
// Descripton:      回调函数声明 


#ifndef DWATER_NET_CALLBACKS_H
#define DWATER_NET_CALLBACKS_H

#include "dwater/base/timestamp.h"

#include <functional>
#include <memory>

namespace dwater {

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;


// get native pointer for any type
template<typename T>
inline T* GetPointer(const std::shared_ptr<T>& ptr) {
    return ptr.get();
}

template<typename T>
inline T* GetPointer(const std::unique_ptr<T>& ptr) {
    return ptr.get();
}

template<typename To, typename From>
inline ::std::shared_ptr<To>  DownPointetCast(const ::std::shared_ptr<From>& f) {
    if ( false ) {
        implicit_cast<From*, To*>(0);
    }

#ifndef NDEBUG
    assset(f == NULL || dynamic_cast<To*>(GetPointer(f)) != NULL);
#endif
    return ::std::static_pointer_cast<To>(f);
}

namespace net {

// all client callbacks

class                                                       Buffer;
class                                                       TcpConnection;
typedef std::shared_ptr<TcpConnection>                      TcpConnectionPtr;
typedef std::function<void()>                               TimerCallback;
typedef std::function<void (const TcpConnectionPtr&)>       TcpConnectionCallback;
typedef std::function<void (const TcpConnectionPtr&)>       CloseCallback;
typedef std::function<void (const TcpConnectionPtr&)>       WriteCompleteCallback;
typedef std::function<void (const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;       


typedef std::function<void (const TcpConnectionPtr&,
                            Buffer*,
                            Timestamp)> MessageCallback;

void DefaultConnectionCallback(const TcpConnectionPtr& conn);
void DefaultMessageCallback(const TcpConnectionPtr& conn,
                            Buffer* buffer,
                            Timestamp receive_time);
} // namespace net;

} // namespace dwater

#endif  // DWATER_NET_CALLBACKS_H
