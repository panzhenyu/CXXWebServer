#include <cstring>
#include <sstream>
#include "HttpResponse.hpp"
#include "ResourceAccessor.hpp"

#define BFSIZE  4096

static char _local_static_buff[BFSIZE];

HttpVersion HttpResponse::getHttpVersion() const { return __version; }

const HttpResponseStatus& HttpResponse::getResponseStatus() const { return __status; }

const HttpResponse::header_t& HttpResponse::getHeader() const { return __header; }

HttpResponse::body_t HttpResponse::getBody() const { return __body; }

void HttpResponse::setHttpVersion(HttpVersion _v) { __version = _v; }

void HttpResponse::setResponseStatus(statecode_t _code) {
    if (_code != __status.getCode()) __status = HttpResponseStatus(_code);
}

void HttpResponse::setHeader(const key_t& _k, const value_t& _v) { __header[_k] = _v; }

void HttpResponse::delHeader(const key_t& _k) {
    if (__header.count(_k)) __header.erase(_k);
}

void HttpResponse::setBody(body_t _body) { __body = _body; }

HttpResponseBuilder::HttpResponseBuilder(): 
    __obj(std::shared_ptr<HttpResponse>(new HttpResponse)) {}

HttpResponseBuilder::response_sptr_t HttpResponseBuilder::build() { return __obj; }

IHttpResponseBuilder& HttpResponseBuilder::setHttpVersion(HttpVersion _v) {
    __obj->setHttpVersion(_v);
    return *this;
}

IHttpResponseBuilder& HttpResponseBuilder::setResponseStatus(statecode_t _code) {
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

// use router get coresponding resource, return a status setted HttpResponse pointer
HttpResponsor::response_sptr_t HttpResponsor::getResponseFromRequest(HttpRequest& _request) const {
    server_err_t err;
    statecode_t code;
    HttpVersion version;
    HttpRequestMethod method;
    HttpResponseBuilder builder;
    HttpResponse::body_t staticBody;
    std::shared_ptr<GeneralResource> resource;
    Router& router = Router::getRouter();

    version = _request.getHttpVersion();
    method = _request.getRequestMethod();
    resource = router[_request.getURI()];

    // must set error pages
    if (version == UNKNOWN_HTTP_VERSION) { version = HTTP_1_0; code = 505; }
    else if (method == UNKNOWN_REQUEST_METHOD) code = 501;
    else {
        switch (resource->getType()) {
            case GeneralResource::STATIC:
                staticBody = LocalResourceAccessor::access(static_cast<StaticResource&>(*resource), err);
                if (SERVER_OK == err) goto buildObj;
                code = 500; break;
            case GeneralResource::CGI:
                // not complete
                goto buildObj;
            case GeneralResource::INVALID:
                code = 404; break;
        }
    }
    // try to get error page body, if error page isn't setted, return a default error page
    if ((resource=router[code])->getType() != GeneralResource::INVALID)
        staticBody = LocalResourceAccessor::access(static_cast<StaticResource&>(*resource), err);
    if (err != SERVER_OK) staticBody = router.getDefaultErrorPage(code);

buildObj:
    return builder.setHttpVersion(version).setResponseStatus(code).setBody(staticBody).build();
}

server_err_t HttpResponsor::genResponseLine(
std::ostream& _output, 
HttpResponse& _response) const {
    auto& responseStatus = _response.getResponseStatus();
    _output << _response.getHttpVersion() << ' ' 
        << responseStatus.getCode() << ' ' 
        << responseStatus.getDetail() << RESPONSE_ENDLINE;
    return _output.fail() ? GEN_RESPONSE_LINE_FAILED : SERVER_OK;
}

server_err_t HttpResponsor::genResponseHead(
std::ostream& _output, 
HttpResponse& _response) const {
    for (auto& [key, value] : _response.getHeader()) {
        _output << key << ": " << value << RESPONSE_ENDLINE;
        if (_output.fail()) return GEN_RESPONSE_HEAD_FAILED;
    }
    return SERVER_OK;
}

server_err_t HttpResponsor::genResponseBody(
std::ostream& _output, 
HttpResponse& _response, long _len) const {
    long rest, cur;
    server_err_t error;
    HttpResponse::body_t body;

    error = SERVER_OK;
    body = _response.getBody(); body->seekg(0, body->beg); rest = _len;
    while (rest > 0) {
        cur = rest < BFSIZE ? rest : BFSIZE;
        if (!body->fail()) cur = body->readsome(_local_static_buff, cur);
        else cur = rest < BFSIZE ? rest : BFSIZE;
        _output.write(_local_static_buff, cur); rest -= cur;

        if (rest>0 && body->fail()) {
            std::memset(_local_static_buff, 0, 4096);
            error = GEN_RESPONSE_BODY_FAILED;
        }
        if (_output.fail()) { error = GEN_RESPONSE_BODY_FAILED; break; }
    }
    return error;
}

// serialize HttpResponse to output, use a ResourceAccessor to access concrete response
server_err_t HttpResponsor::response(
std::ostream& _output, 
HttpResponse& _response) const {
    server_err_t error;
    std::streampos bodyLen;
    HttpResponse::body_t body;

    if (SERVER_OK!=(error=genResponseLine(_output, _response)) || _output.fail()) goto out;

    // set Content-Length before generate head
    body = _response.getBody(); body->seekg(0, body->end); bodyLen = body->tellg(); body->seekg(0, body->beg);
    if (bodyLen != -1) _response.setHeader("Content-Length", to_string(bodyLen));
    else { error = INVALID_CONTENT_LENGTH; goto out; }
    if (SERVER_OK!=(error=genResponseHead(_output, _response)) || _output.fail()) goto out;

    if ((_output<<RESPONSE_ENDLINE).fail()) { error = GEN_RESPONSE_BODY_FAILED; goto out; }
    error = genResponseBody(_output, _response, (long)bodyLen);
out:
    _output.flush();
    return error;
}

std::ostream& operator<<(std::ostream& _output, HttpResponse& _response) {
    HttpResponsor().response(_output, _response);
    return _output;
}
