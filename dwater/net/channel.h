// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.03.22
// Filename:        channel.h
// Descripton:      Channel是事件，每个Channel对象只属于一个EventLoop，因此
// 只属于一个IO线程，只负责一个文件描述符，但是并不拥有这个负责的fd，也不会在析构
// 的时候关闭这个fd。Channel会把不同的IO事件分发为不同的回调，比如ReadCallback、
// WriteCallback等。Channel一般会封装为其他Class的直接成员或者间接成员。
// 封装文件描述符以及关注的事件类型

#ifndef DWATER_NET_CHANNEL_H
#define DWATER_NET_CHANNEL_H

#include "dwater/base/noncopable.h"
#include "dwater/base/timestamp.h"

#include <functional>
#include <memory>

namespace dwater {

namespace net {

class EventLoop;

class Channel : noncopyable {
public:
    typedef std::function<void()>           EventCallback;
    typedef std::function<void(Timestamp)>  ReadEventCallback;
    
    Channel(EventLoop* loop, int fd);
    ~Channel();

    // 由Loop()调用，根据revents_的值分别调用不同的回调函数
    void HandleEvent(Timestamp receive_time);

    // 设置读、写、关闭、错误的回调函数
    void SetReadCallback(ReadEventCallback cb) {
        read_callback_ = std::move(cb);
    }
    void SetWriteCallback(EventCallback cb) {
        write_callback_ = std::move(cb);
    }
    void SetCloseCallback(EventCallback cb) {
        close_callback_ = std::move(cb);
    }
    void SetErrorCallback(EventCallback cb) {
        error_callback_ = std::move(cb);
    }

    void Tie(const std::shared_ptr<void>&);

    // 拥有的fd
    int Fd() const {
        return fd_;
    }

    // 当前处理的事件类型
    int Events() const {
        return events_;
    }

    // 
    void SetRevents(int revt) {
        revents_ = revt;
    }

    // 是否没有处理任何事件
    bool IsNoneEvent() const {
        return events_ == knone_event;
    }

    void EnableReading() {
        events_ |= kread_event;
        Update();
    }

    void DisableReading() {
        events_ &= ~kread_event;
        Update();
    }

    void EnableWriting() {
        events_ |= kwrite_event;
        Update();
    }

    void DisableWriting() {
        events_ &= ~kwrite_event;
        Update();
    }

    void DisableAll() {
        events_ = knone_event;
        Update();
    }

    bool IsWriting() const {
        return events_ & kwrite_event;
    }

    bool IsReading() const {
        return events_ & kread_event;
    }

    int Index() {
        return index_;
    }

    void SetIndex(int index) {
        index_ = index;
    }

    string ReventsToString() const;
    string EventsToString() const;

    void DoNotLogHup() {
        log_hup_ = false;
    }

    EventLoop* OwnerLoop() {
        return loop_;
    }

    void Remove();
private:
    static string EventsToString(int fd, int event);

    void Update();
    void HandleEventWithGuard(Timestamp receive_time);

    static const int        knone_event;
    static const int        kread_event;
    static const int        kwrite_event;

    EventLoop*              loop_;      // 当前Channel属于的EventLoop 
    const int               fd_;        // 所负责的文件描述符
    int                     events_;    // Channel关心的事件
    int                     revents_;   // 目前活动的事件
    int                     index_;     // 在Poller事件数组中的索引
    bool                    log_hup_;   // 

    std::weak_ptr<void>     tie_;       // 负责控制当前Channel的生存期
    bool                    tied_;
    bool                    event_handling_;
    bool                    added_to_loop_;
    // 几种回调函数
    ReadEventCallback       read_callback_;
    EventCallback           write_callback_;
    EventCallback           close_callback_;
    EventCallback           error_callback_;
}; // class Channel

} // namespace dwater

} // namespace dwater 

#endif // DWATER_NET_CHANNEL_H
