#pragma once

#include "Router.hpp"
#include "HttpRequest.hpp"
#include "SocketStream.hpp"

class HttpResponse {
    friend class HttpResponseBuilder;
public:
    using key_t     = std::string;
    using value_t   = std::string;
    using header_t  = std::unordered_map<key_t, value_t>;
    using body_t    = std::shared_ptr<std::istream>;
private:
    HttpResponse()  = default;
public:
    HttpVersion         getHttpVersion();
    HttpResponseStatus& getResponseStatus();
    header_t&           getHeader();
    body_t              getBody();

    void setHttpVersion(HttpVersion);
    void setResponseStatus(statecode_t);
    void setHeader(const key_t&, const value_t&);
    void delHeader(const key_t&);
    void setBody(body_t);
private:
    HttpVersion         __version;
    HttpResponseStatus  __status;
    header_t            __header;
    body_t              __body;     // a lazy body, need ResourceAccessor to serailize
};

struct IHttpResponseBuilder {
    using key_t             = HttpResponse::key_t;
    using value_t           = HttpResponse::value_t;
    using body_t            = HttpResponse::body_t;
    using response_sptr_t   = std::shared_ptr<HttpResponse>;

    IHttpResponseBuilder() = default;
    virtual ~IHttpResponseBuilder() = 0;
    virtual response_sptr_t build() = 0;
    virtual IHttpResponseBuilder& setHttpVersion(HttpVersion) = 0;
    virtual IHttpResponseBuilder& setResponseStatus(statecode_t) = 0;
    virtual IHttpResponseBuilder& setHeader(const key_t&, const value_t&) = 0;
    virtual IHttpResponseBuilder& delHeader(const key_t&) = 0;
    virtual IHttpResponseBuilder& setBody(const body_t&) = 0;
};

class HttpResponseBuilder: public IHttpResponseBuilder {
public:
    HttpResponseBuilder();
    HttpResponseBuilder(const HttpResponseBuilder&) = delete;
    ~HttpResponseBuilder() = default;
    virtual response_sptr_t build();
    virtual IHttpResponseBuilder& setHttpVersion(HttpVersion);
    virtual IHttpResponseBuilder& setResponseStatus(statecode_t);
    virtual IHttpResponseBuilder& setHeader(const key_t&, const value_t&);
    virtual IHttpResponseBuilder& delHeader(const key_t&);
    virtual IHttpResponseBuilder& setBody(const body_t&);
private:
    response_sptr_t __obj;
};

class HttpResponsor {
public:
    using response_sptr_t   = std::shared_ptr<HttpResponse>;
private:
    server_err_t genResponseLine(std::ostream&, HttpResponse&);
    server_err_t genResponseHead(std::ostream&, HttpResponse&);
    server_err_t genResponseBody(std::ostream&, HttpResponse&, long);
public:
    HttpResponsor() = default;
    HttpResponsor(const HttpResponsor&) = delete;
    ~HttpResponsor() = default;
    response_sptr_t getResponseFromRequest(HttpRequest&);
    server_err_t response(std::ostream&, HttpResponse&);
};

std::ostream& operator<<(std::ostream&, HttpResponse&);
