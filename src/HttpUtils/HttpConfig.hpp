#pragma once

#include <string>
#include <ostream>
#include <unordered_map>

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
