#include "HttpRequest.hpp"

HttpRequestMethod HttpRequest::getRequestMethod() { return __method; }

HttpRequest::uri_t& HttpRequest::getURI() { return __uri; }

HttpVersion HttpRequest::getHttpVersion() { return __version; }

HttpRequest::header_t& HttpRequest::getHeader() { return __header; }

HttpRequest::body_t& HttpRequest::getBody() { return __body; }

void HttpRequest::setRequestMethod(HttpRequestMethod _m) { __method = _m; }

void HttpRequest::setHttpVersion(HttpVersion _v) { __version = _v; }

void HttpRequest::setURI(HttpRequest::uri_t& _uri) { __uri = _uri; }

void HttpRequest::setHeader(HttpRequest::key_t& _k, HttpRequest::value_t& _v) { __header[_k] = _v; }

void HttpRequest::delHeader(HttpRequest::key_t& _k) { __header.erase(_k); }

void HttpRequest::setBody(HttpRequest::body_t& _body) { __body = _body; }

std::string HttpRequest::serialize() {
    return "";
}

HttpRequestBuilder::HttpRequestBuilder(): 
    __obj(std::shared_ptr<HttpRequest>(new HttpRequest())) {}

HttpRequestBuilder::request_sptr_t HttpRequestBuilder::build() { return __obj; }

IHttpReqeustBuilder& HttpRequestBuilder::setRequestMethod(HttpRequestMethod _m) {
    __obj->setRequestMethod(_m);
    return *this;
}

IHttpReqeustBuilder& HttpRequestBuilder::setHttpVersion(HttpVersion _v) {
    __obj->setHttpVersion(_v);
    return *this;
}

IHttpReqeustBuilder& HttpRequestBuilder::setURI(HttpRequest::uri_t& _uri) {
    __obj->setURI(_uri);
    return *this;
}

IHttpReqeustBuilder& HttpRequestBuilder::setHeader(HttpRequest::key_t& _k, HttpRequest::value_t& _v) {
    __obj->setHeader(_k, _v);
    return *this;
}

IHttpReqeustBuilder& HttpRequestBuilder::delHeader(HttpRequest::key_t& _k) {
    __obj->delHeader(_k);
    return *this;
}

IHttpReqeustBuilder& HttpRequestBuilder::setBody(HttpRequest::body_t& _body) {
    __obj->setBody(_body);
    return *this;
}

server_err_t HttpRequestAnalyser::parseLine(HttpRequestBuilder& _builder) {
    HttpVersion v;
    HttpRequestMethod m;
    std::string method, uri, version;

    *__input >> method;
    if (__input->fail() || UNKNOWN_REQUEST_METHOD==(m=toRequestMethod(method))) return PARSE_REQ_METHOD_FAILED;
    *__input >> uri;
    if (__input->fail()) return PARSE_REQ_URI_FAILED;
    *__input >> version;
    if (__input->fail() || UNKNOWN_HTTP_VERSION==(v=toHttpVersion(version))) return PARSE_REQ_VERSION_FAILED;

    _builder.setRequestMethod(m).setURI(uri).setHttpVersion(v);
    return SERVER_OK;
}

server_err_t HttpRequestAnalyser::parseHead(HttpRequestBuilder& _builder) {
    std::string cur;
    *__input >> cur; _builder.setRequestMethod(toRequestMethod(cur));
    return SERVER_OK;
}

server_err_t HttpRequestAnalyser::parseBody(HttpRequestBuilder& _builder) {
    return SERVER_OK;
}

HttpRequestAnalyser::HttpRequestAnalyser(input_uptr_t&& _input): __input(std::move(_input)) {}

HttpRequestAnalyser::request_sptr_t HttpRequestAnalyser::getOneHttpRequest(server_err_t& _err) {
    HttpRequestBuilder reqBuilder;
    if (SERVER_OK != (_err=parseLine(reqBuilder))) goto out;
    if (SERVER_OK != (_err=parseHead(reqBuilder))) goto out;
    if (SERVER_OK != (_err=parseBody(reqBuilder))) goto out;
out:
    return reqBuilder.build();
}
