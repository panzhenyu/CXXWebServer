#include "HttpResponse.hpp"

inline HttpVersion HttpResponse::getHttpVersion() { return __version; }

inline HttpResponseStatus& HttpResponse::getResponseStatus() { return __status; }

inline HttpResponse::header_t& HttpResponse::getHeader() { return __header; }

inline HttpResponse::body_t& HttpResponse::getBody() { return __body; }

// std::string HttpResponse::serialize() { return ""; }

inline void HttpResponse::setHttpVersion(HttpVersion _v) { __version = _v; }

void HttpResponse::setResponseStatus(uint16_t _code) {
    if (_code != __status.__code) __status = HttpResponseStatus(_code);
}

inline void HttpResponse::setHeader(const key_t& _k, const value_t& _v) { __header[_k] = _v; }

void HttpResponse::delHeader(const key_t& _k) {
    if (__header.count(_k)) __header.erase(_k);
}

inline void HttpResponse::setBody(const body_t& _body) { __body = _body; }

HttpResponseBuilder::HttpResponseBuilder(): 
    __obj(std::shared_ptr<HttpResponse>(new HttpResponse)) {}

inline HttpResponseBuilder::response_sptr_t HttpResponseBuilder::build() { return __obj; }

IHttpResponseBuilder& HttpResponseBuilder::setHttpVersion(HttpVersion _v) {
    __obj->setHttpVersion(_v);
    return *this;
}

IHttpResponseBuilder& HttpResponseBuilder::setResponseStatus(uint16_t _code) {
    __obj->setResponseStatus(_code);
    return *this;
}

IHttpResponseBuilder& HttpResponseBuilder::setHeader(const key_t& _k, const value_t& _v) {
    __obj->setHeader(_k, _v);
    return *this;
}

IHttpResponseBuilder& HttpResponseBuilder::delHeader(const key_t& _k) {
    __obj->delHeader(_k);
    return *this;
}

IHttpResponseBuilder& HttpResponseBuilder::setBody(const body_t& _body) {
    __obj->setBody(_body);
    return *this;
}

HttpResponsor::HttpResponsor(const Router& _router): __router(_router) {}

// use router get coresponding resource, return a status setted HttpResponse pointer
HttpResponsor::response_sptr_t HttpResponsor::getResponseFromRequest(request_sptr_t _request) {
    HttpResponseBuilder builder;
    HttpRequestMethod method;
    HttpVersion version;
    Resource resource;

    method = _request->getRequestMethod();
    version = _request->getHttpVersion();
    resource = __router[_request->getURI()];
    // must set error pages
    if (version == UNKNOWN_HTTP_VERSION) {
        return builder
            .setHttpVersion(HTTP_1_0)
            .setResponseStatus(505)
            .setBody(Router::getRouter()[std::string(DEFAULT_ERROR_ROOT)+"505.html"])
            .build();
    } else builder.setHttpVersion(version);
    if (method == UNKNOWN_REQUEST_METHOD) {
        return builder
            .setResponseStatus(501)
            .setBody(Router::getRouter()[std::string(DEFAULT_ERROR_ROOT)+"501.html"])
            .build();
    }
    if (resource.__type == Resource::UNKNOWN) {
        return builder
            .setResponseStatus(404)
            .setBody(Router::getRouter()[std::string(DEFAULT_ERROR_ROOT)+"404.html"])
            .build();
    }
    return builder.setResponseStatus(200).setBody(resource).build();
}

// serialize HttpResponse to output, use a ResourceAccessor to access concrete response
server_err_t HttpResponsor::response(
response_sptr_t _response, 
output_uptr_t _output) {
    server_err_t error;

    error = SERVER_OK;
    return error;
}
