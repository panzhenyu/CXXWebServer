#pragma once

#include <string>
#include <ostream>

enum HttpRequestMethod {
    GET, HEAD, POST, 
    PUT, DELETE, CONNECT, 
    OPTIONS, TRACE, PATCH,
    UNKNOWN_REQUEST_METHOD
};
HttpRequestMethod toRequestMethod(std::string&);
std::string serialize(HttpRequestMethod);
std::ostream& operator<<(std::ostream&, HttpRequestMethod);

enum HttpVersion {
    HTTP_1_0, HTTP_1_1, 
    UNKNOWN_HTTP_VERSION
};
HttpVersion toHttpVersion(std::string&);
std::string serialize(HttpVersion);
std::ostream& operator<<(std::ostream&, HttpVersion);

struct HttpResponseStatus {
    uint16_t    __code;
    std::string __detail;
    HttpResponseStatus();
    HttpResponseStatus(uint16_t _code);
    HttpResponseStatus(const HttpResponseStatus&) = delete;
    ~HttpResponseStatus() = default;
};
