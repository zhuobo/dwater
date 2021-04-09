// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.04.09
// Filename:        http_request.h
// Descripton:       

#ifndef DWATER_NET_HTTP_HTTPREQUEST_H
#define DWATER_NET_HTTP_HTTPREQUEST_H

#include "dwater/base/copyable.h"
#include "dwater/base/timestamp.h"
#include "dwater/base/types.h"

#include <map>
#include <assert.h>
#include <stdio.h>

namespace dwater {
namespace net {

class HttpRequest : public dwater::copyable {
public:
    enum Method {
        kinvalid,
        kget,
        kpost,
        khead,
        kput,
        kdelete
    };

    enum Version {
        kunknow,
        khttp10,
        khttp11
    };

    HttpRequest() : method_(kinvalid), version_(kunknow) {

    }

    void SetVersion(Version version) {
        version_ = version;
    }

    Version GetVersion() const {
        return version_;
    }

    bool SetMethod(const char* start, const char* end) {
        assert(method_ == kinvalid);
        string m(start, end);
        if ( m == "GET" ) {
            method_ = kget;
        } else if ( m == "POST" ) {
            method_ = kpost;
        } else if ( m == "HEAD" ) {
            m = khead;
        } else if ( m == "PUT" ) {
            method_ = kput;
        } else if ( m == "DELETE" ) {
            method_ = kdelete;
        } else {
            method_ = kinvalid;
        }
        return method_ != kinvalid;
    }

    Method GetMethod() const {
        return method_;
    }

    const char* MethodString() const {
        const char* result = "UNKNOWN";
        switch(method_) {
        case kget:
            result = "GET";
            break;
        case kpost:
            result = "POST";
            break;
        case khead:
            result = "HEAD";
            break;
        case kput:
            result = "PUT";
            break;
        case kdelete:
            result = "DELETE";
            break;
        default:
            break;
        }
        return result;
    }

    void SetPath(const char* start, const char* end) {
        path_.assign(start, end);
    }
    
    const string& GetPath() const {
        return path_;
    }

    void SetQuery(const char* start, const char* end) {
        query_.assign(start, end);
    }

    const string& GetQuery() const {
        return query_;
    }

    void SetReceiveTime(Timestamp t) {
        receive_time_ = t;
    }

    Timestamp GetReceiveTime() const {
        return receive_time_;
    }

    void AddHeader(const char* start, const char* colon, const char* end) {
        string field(start, colon);
        ++colon;
        while ( colon < end && isspace(*colon) ) {
            ++colon;
        }
        string value(colon, end);
        while ( !value.empty() && isspace(value[value.size() - 1]) ) {
            value.resize(value.size() - 1);
        }
        headers_[field] = value;
    }

    string GetHeader(const string& field) const {
        string result;
        std::map<string, string>::const_iterator iter = headers_.find(field);
        if ( iter != headers_.end() ) {
            result = iter->second;
        }
        return result;;
    }

    const std::map<string, string>& GetHeaders() const {
        return headers_;
    }

    void Swap(HttpRequest& that) {
        std::swap(method_, that.method_);
        std::swap(version_, that.version_);
        path_.swap(that.path_);
        query_.swap(that.query_);
        receive_time_.swap(that.receive_time_);
        headers_.swap(that.headers_);
    }
private:
    Method                      method_;
    Version                     version_;
    string                      path_;
    string                      query_;
    Timestamp                   receive_time_;
    std::map<string, string>    headers_;
}; // class HttpRequest

} // namespace net;
} // namespace dwater

#endif // DWATER_NET_HTTP_HTTPREQUEST_H
