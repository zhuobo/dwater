// Author:          Drinkwater
// Email:           tanzhuobo@gmail.com
// Last modified:   2021.04.09
// Filename:        http_response.cc
// Descripton:       

#include "dwater/net/http/http_response.h"

#include "dwater/net/buffer.h"

#include <stdio.h>

using namespace dwater;
using namespace dwater::net;

void HttpResponse::AppendToBuffer(Buffer* output) const {
    char buf[32];
    snprintf(buf, sizeof(buf), "HTTP/1.1 %d ", status_code_);
    output->Append(buf);
    output->Append(status_message_);
    output->Append("\r\n");

    if ( close_connection_ ) {
        output->Append("Connection: close\r\n");
    } else {
        snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", body_.size());
        output->Append(buf);
        output->Append("Connection: Keep-Alive\r\n");
    }

    for ( const auto& header : headers_ ) {
        output->Append(header.first);
        output->Append(": ");
        output->Append(header.second);
        output->Append("\r\n");
    }
    output->Append("\r\n");
    output->Append(body_);
}
