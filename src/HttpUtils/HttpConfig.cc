#include "HttpConfig.hpp"

static const char* HttpRequestMethodString[] = {
    "GET", "HEAD", "POST", 
    "PUT", "DELETE", "CONNECT", 
    "OPTIONS", "TRACE", "PATCH"
};
static std::unordered_map<std::string, HttpRequestMethod> string2HttpReqeustMethod = {
    {"GET", HttpRequestMethod::GET}, {"HEAD", HttpRequestMethod::HEAD}, {"POST", HttpRequestMethod::POST}, 
    {"PUT", HttpRequestMethod::PUT}, {"DELETE", HttpRequestMethod::DELETE}, {"CONNECT", HttpRequestMethod::CONNECT}, 
    {"OPTIONS", HttpRequestMethod::OPTIONS}, {"TRACE", HttpRequestMethod::TRACE}, {"PATCH", HttpRequestMethod::PATCH}
};

static const char* HttpVersionString[] = { "HTTP/1.0", "HTTP/1.1" };
static std::unordered_map<std::string, HttpVersion> string2HttpVersion = {
    {"HTTP/1.0", HttpVersion::HTTP_1_0}, {"HTTP/1.1", HttpVersion::HTTP_1_1}
};

HttpRequestMethod toRequestMethod(std::string& _s) {
    return string2HttpReqeustMethod.count(_s) ? HttpRequestMethod::UNKNOWN_REQUEST_METHOD : string2HttpReqeustMethod[_s];
}
std::string serialize(HttpRequestMethod _m) { return std::string(HttpRequestMethodString[_m]); }
std::ostream& operator<<(std::ostream& _o, HttpRequestMethod _m) { return _o << serialize(_m); }

HttpVersion toHttpVersion(std::string& _s) {
    return string2HttpVersion.count(_s) ? HttpVersion::UNKNOWN_HTTP_VERSION : string2HttpVersion[_s];
}
std::string serialize(HttpVersion _v) { return HttpVersionString[_v]; }
std::ostream& operator<<(std::ostream& _o, HttpVersion _v) { return _o << serialize(_v); }
