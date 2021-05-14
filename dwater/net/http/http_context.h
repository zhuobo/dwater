// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.04.09
// Filename:        http_context.h
// Descripton:       

#ifndef DWATER_NET_HTTP_HTTP_CONTEXT_H
#define DWATER_NET_HTTP_HTTP_CONTEXT_H

#include "dwater/base/copyable.h"

#include "dwater/net/http/http_request.h"

namespace dwater {
namespace net {

class Buffer;

class HttpContext : public dwater::copyable {
public:
    enum HttpRequestParseState {
        kexpect_request_line,
        kexpect_headers,
        kexpect_body,
        k_got_all,
    };

    HttpContext() : state_(kexpect_request_line) {  }

    bool ParseRequest(Buffer* buf, Timestamp receive_time);

    bool GotAll() const {
        return state_ == k_got_all;
    }

    void Reset() {
        state_ = kexpect_request_line;
        HttpRequest dummy;
        request_.Swap(dummy);
    }

    const HttpRequest& Requeset() const {
        return request_;
    }

    HttpRequest& Requeset() {
        return request_;
    }

private:
    bool ProcessRequestLine(const char* begin, const char* end);

    HttpRequestParseState state_;
    HttpRequest           request_;
};
}
}

#endif // DWATER_NET_HTTP_HTTP_CONTEXT_H 
