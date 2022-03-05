#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "../Error.hpp"
#include "HttpConfig.hpp"
#include "SocketStream.hpp"

class HttpRequest {
public:
    using uri_t     = std::string;
    using key_t     = std::string;
    using value_t   = std::string;
    using header_t  = std::unordered_map<key_t, value_t>;
    using body_t    = std::string;
private:
    HttpRequest();
public:
    ~HttpRequest() = default;
    
    HttpRequestMethod getRequestMethod();
    uri_t& getURI();
    HttpVersion getHttpVersion();
    header_t& getHeader();
    body_t& getBody();

    void setRequestMethod(HttpRequestMethod);
    void setHttpVersion(HttpVersion);
    void setURI(const uri_t&);
    void setHeader(const key_t&, const value_t&);
    void delHeader(const key_t&);
    void setBody(const body_t&);

    std::string serialize();
private:
    friend class HttpRequestBuilder;
    HttpRequestMethod   __method;
    uri_t               __uri;
    HttpVersion         __version;
    header_t            __header;
    body_t              __body;
};

struct IHttpReqeustBuilder {
    using request_sptr_t = std::shared_ptr<HttpRequest>;
    virtual request_sptr_t build() = 0;
    virtual ~IHttpReqeustBuilder() = 0;
    virtual IHttpReqeustBuilder& setRequestMethod(HttpRequestMethod) = 0;
    virtual IHttpReqeustBuilder& setHttpVersion(HttpVersion) = 0;
    virtual IHttpReqeustBuilder& setURI(HttpRequest::uri_t&) = 0;
    virtual IHttpReqeustBuilder& setHeader(HttpRequest::key_t&, HttpRequest::value_t&) = 0;
    virtual IHttpReqeustBuilder& delHeader(HttpRequest::key_t&) = 0;
    virtual IHttpReqeustBuilder& setBody(HttpRequest::body_t&) = 0;
};

class HttpRequestBuilder: public IHttpReqeustBuilder {
public:
    HttpRequestBuilder();
    HttpRequestBuilder(const HttpRequestBuilder&) = delete;
    virtual ~HttpRequestBuilder() = default;
    virtual request_sptr_t build();
    virtual IHttpReqeustBuilder& setRequestMethod(HttpRequestMethod);
    virtual IHttpReqeustBuilder& setHttpVersion(HttpVersion);
    virtual IHttpReqeustBuilder& setURI(HttpRequest::uri_t&);
    virtual IHttpReqeustBuilder& setHeader(HttpRequest::key_t&, HttpRequest::value_t&);
    virtual IHttpReqeustBuilder& delHeader(HttpRequest::key_t&);
    virtual IHttpReqeustBuilder& setBody(HttpRequest::body_t&);
private:
    request_sptr_t  __obj;
};

class HttpRequestAnalyser {
public:
    using request_sptr_t    = IHttpReqeustBuilder::request_sptr_t;
    using input_uptr_t      = std::unique_ptr<SocketInputStream>;
private:
    bool haveRequestBody(request_sptr_t);
    server_err_t skipTerminateCH();
    server_err_t parseLine(HttpRequestBuilder&);
    server_err_t parseHead(HttpRequestBuilder&);
    server_err_t parseBody(HttpRequestBuilder&);
public:
    HttpRequestAnalyser(input_uptr_t&&);
    HttpRequestAnalyser(const HttpRequestAnalyser&) = delete;
    ~HttpRequestAnalyser() = default;
    // for some error(.e.g unsupported http version), request should be returned normally
    request_sptr_t getOneHttpRequest(server_err_t&);
private:
    input_uptr_t __input;
};

std::ostream operator<<(std::ostream&, HttpRequest&);
