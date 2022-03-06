#pragma once

#include <string>
#include <ostream>

#define DEFAULT_MAPPING_PATH    "web.xml"
#define RESPONSE_ENDLINE '\n'

enum HttpRequestMethod {
    GET, HEAD, POST, 
    PUT, DELETE, CONNECT, 
    OPTIONS, TRACE, PATCH,
    UNKNOWN_REQUEST_METHOD
};
HttpRequestMethod toRequestMethod(const std::string&);
std::string serialize(HttpRequestMethod);
std::ostream& operator<<(std::ostream&, HttpRequestMethod);

enum HttpVersion {
    HTTP_1_0, HTTP_1_1, 
    UNKNOWN_HTTP_VERSION
};
HttpVersion toHttpVersion(const std::string&);
std::string serialize(HttpVersion);
std::ostream& operator<<(std::ostream&, HttpVersion);

typedef uint16_t statecode_t;
struct HttpResponseStatus {
    HttpResponseStatus();
    HttpResponseStatus(statecode_t _code);
    HttpResponseStatus(const HttpResponseStatus&) = delete;
    ~HttpResponseStatus() = default;
    statecode_t getCode() const;
    const std::string& getDetail() const;
private:
    statecode_t __code;
    std::string __detail;
};
