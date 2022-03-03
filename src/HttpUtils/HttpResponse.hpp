#pragma once

#include "Router.hpp"
#include "HttpRequest.hpp"
#include "SocketStream.hpp"

class HttpResponse {

};

struct IHttpResponseBuilder {

};

class HttpResponseBuilder: public HttpResponse {

};

class HttpResponsor {
public:
    using output_uptr_t     = std::unique_ptr<SocketOutputStream>;
    using request_sptr_t    = std::shared_ptr<HttpRequest>;
    using response_sptr_t   = std::shared_ptr<HttpResponse>;
public:
    response_sptr_t response(request_sptr_t);
private:
    output_uptr_t   __output;
};
