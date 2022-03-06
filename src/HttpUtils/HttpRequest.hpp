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
    const uri_t& getURI() const;
    HttpVersion getHttpVersion() const;
    const header_t& getHeader() const;
    const body_t& getBody() const;

    void setRequestMethod(HttpRequestMethod);
    void setHttpVersion(HttpVersion);
    void setURI(const uri_t&);
    void setHeader(const key_t&, const value_t&);
    void delHeader(const key_t&);
    void setBody(const body_t&);

    std::string serialize() const;
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
protected:
    IHttpReqeustBuilder() = default;
public:
    virtual ~IHttpReqeustBuilder() = default;
    virtual request_sptr_t build() = 0;
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
    using request_sptr_t = IHttpReqeustBuilder::request_sptr_t;
private:
    size_t requestBodyLength(request_sptr_t);
    server_err_t skipTerminateCH(std::istream&);
    server_err_t parseLine(std::istream&, HttpRequestBuilder&);
    server_err_t parseHead(std::istream&, HttpRequestBuilder&);
    server_err_t parseBody(std::istream&, HttpRequestBuilder&);
public:
    HttpRequestAnalyser() = default;
    HttpRequestAnalyser(const HttpRequestAnalyser&) = delete;
    ~HttpRequestAnalyser() = default;
    // for some error(.e.g unsupported http version), request should be returned normally
    request_sptr_t getOneHttpRequest(std::istream&, server_err_t&);
};

std::ostream& operator<<(std::ostream&, const HttpRequest&);
