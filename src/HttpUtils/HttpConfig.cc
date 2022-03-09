#include <unordered_map>
#include "HttpConfig.hpp"

static const char* HttpRequestMethodString[] = {
    "GET", "HEAD", "POST", 
    "PUT", "DELETE", "CONNECT", 
    "OPTIONS", "TRACE", "PATCH", 
    "UNKNOWN_REQUEST_METHOD"
};

static std::unordered_map<std::string, HttpRequestMethod> string2HttpReqeustMethod = {
    {"GET", HttpRequestMethod::GET}, {"HEAD", HttpRequestMethod::HEAD}, {"POST", HttpRequestMethod::POST}, 
    {"PUT", HttpRequestMethod::PUT}, {"DELETE", HttpRequestMethod::DELETE}, {"CONNECT", HttpRequestMethod::CONNECT}, 
    {"OPTIONS", HttpRequestMethod::OPTIONS}, {"TRACE", HttpRequestMethod::TRACE}, {"PATCH", HttpRequestMethod::PATCH}
};

HttpRequestMethod toRequestMethod(const std::string& _s) {
    return string2HttpReqeustMethod.count(_s) ? string2HttpReqeustMethod[_s] :HttpRequestMethod::UNKNOWN_REQUEST_METHOD;
}

std::string serialize(HttpRequestMethod _m) { return HttpRequestMethodString[_m]; }

std::ostream& operator<<(std::ostream& _o, HttpRequestMethod _m) { return _o << serialize(_m); }

static const char* HttpVersionString[] = { "HTTP/1.0", "HTTP/1.1", "UNKNOWN_HTTP_VERSION" };

static std::unordered_map<std::string, HttpVersion> string2HttpVersion = {
    {"HTTP/1.0", HttpVersion::HTTP_1_0}, {"HTTP/1.1", HttpVersion::HTTP_1_1}
};

HttpVersion toHttpVersion(const std::string& _s) {
    return string2HttpVersion.count(_s) ? string2HttpVersion[_s] : HttpVersion::UNKNOWN_HTTP_VERSION;
}

std::string serialize(HttpVersion _v) { return HttpVersionString[_v]; }

std::ostream& operator<<(std::ostream& _o, HttpVersion _v) { return _o << serialize(_v); }

static std::unordered_map<statecode_t, std::string> StatusCodeDetail = {
    {200, "OK"}, {206, "Partial Content"}, 
    {400, "Bad Request"}, {403, "Forbidden"}, {404, "Not Found"}, 
    {500, "Internal Server Error"}, {501, "Not Implemented"}, 
    {502, "Bad Gateway"}, {505, "HTTP Version not supported"}, 
};

HttpResponseStatus::HttpResponseStatus(): HttpResponseStatus(200) {}

HttpResponseStatus::HttpResponseStatus(statecode_t _code): __code(_code) {
    if (StatusCodeDetail.count(__code)) __detail = StatusCodeDetail[__code];
    else __detail = "";
}

statecode_t HttpResponseStatus::getCode() const { return __code; }

const std::string& HttpResponseStatus::getDetail() const { return __detail; }
