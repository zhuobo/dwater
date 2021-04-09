// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.04.09
// Filename:        http_response.h
// Descripton:      

#ifndef DWATER_NET_HTTP_HTTP_RESPONSE_H
#define DWATER_NET_HTTP_HTTP_RESPONSE_H

#include "dwater/base/copyable.h"
#include "dwater/base/types.h"

#include <map>

namespace dwater {
namespace net {
class Buffer;

class HttpResponse : public dwater::copyable {
public:
    enum HttpStatusCode {
        kunknown,
        k200Ok = 200,
        K301MovedPermanpently = 301,
        k400BadRequest = 400,
        k404NotFound = 404,
    };

    explicit HttpResponse(bool close) : status_code_(kunknown), close_connection_(close) {

    }

    void SetStatusCode(HttpStatusCode code) {
        status_code_ = code;
    }

    void SetStatusMessage(const string& message) {
        status_message_ = message;
    }

    void SetCloseConnection(bool on) {
        close_connection_ = on;
    }

    bool CloseConnection() const {
        return close_connection_;
    }

    void SetContentType(const string& content_type) {
        AddHeader("Content-Type", content_type); 
    }

    // FIXME: StringPiece
    void AddHeader(const string& key, const string& value) {
        headers_[key] = value;
    }

    void SetBody(const string& body) {
        body_ = body;
    }

    void AppendToBuffer(Buffer* output) const;

private:
    std::map<string, string> headers_;
    HttpStatusCode           status_code_;
    string                   status_message_;
    bool                     close_connection_;
    string                   body_;
};

}
}

#endif // DWATER_NET_HTTP_HTTP_RESPONSE_H
