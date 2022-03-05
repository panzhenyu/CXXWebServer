#include <cstring>
#include <sstream>
#include "HttpResponse.hpp"
#include "ResourceAccessor.hpp"

static char buff[4096];

HttpVersion HttpResponse::getHttpVersion() { return __version; }

HttpResponseStatus& HttpResponse::getResponseStatus() { return __status; }

HttpResponse::header_t& HttpResponse::getHeader() { return __header; }

HttpResponse::body_t HttpResponse::getBody() { return __body; }

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
HttpResponsor::response_sptr_t HttpResponsor::getResponseFromRequest(HttpRequest& _request) {
    server_err_t err;
    statecode_t code, o;
    HttpVersion version;
    HttpRequestMethod method;
    HttpResponseBuilder builder;
    std::shared_ptr<GeneralResource> resource;
    HttpResponse::body_t staticBody;
    CGIResourceAccessor::handler_t cgiHandler;
    const Router& router = Router::getRouter();

    version = _request.getHttpVersion();
    method = _request.getRequestMethod();
    resource = router[_request.getURI()];

    // must set error pages
    if (version == UNKNOWN_HTTP_VERSION) { version = HTTP_1_0; code = 505; }
    else if (method == UNKNOWN_REQUEST_METHOD) code = 501;
    else {
        switch (resource->getType()) {
            case GeneralResource::STATIC:
                staticBody = StaticResourceAccessor::getStaticResourceAccessor()
                    .access(static_cast<StaticResource&>(*resource), code, err);
                if (SERVER_OK == err) goto buildObj;
                break;
            case GeneralResource::CGI:
                // not complete
                goto buildObj;
            case GeneralResource::INVALID:
                code = 404; break;
        }
    }
    // try to get error page body, if error page isn't setted, return a default error page
    resource = router[code];
    staticBody = StaticResourceAccessor::getStaticResourceAccessor()
        .access(static_cast<StaticResource&>(*resource), o, err);
    if (err != SERVER_OK) staticBody = router.getDefaultErrorPage(code);

buildObj:
    return builder.setHttpVersion(version).setResponseStatus(code).setBody(staticBody).build();
}

server_err_t HttpResponsor::genResponseLine(
std::ostream& _output, 
HttpResponse& _response) {
    auto& responseStatus = _response.getResponseStatus();
    _output << _response.getHttpVersion() << ' ' 
        << responseStatus.getCode() << ' ' 
        << responseStatus.getDetail() << RESPONSE_ENDLINE;
    return _output.fail() ? SERVER_OK : GEN_RESPONSE_LINE_FAILED;
}

server_err_t HttpResponsor::genResponseHead(
std::ostream& _output, 
HttpResponse& _response) {
    for (auto& [key, value] : _response.getHeader()) {
        _output << key << ": " << value << RESPONSE_ENDLINE;
        if (_output.fail()) return GEN_RESPONSE_HEAD_FAILED;
    }
    return SERVER_OK;
}

server_err_t HttpResponsor::genResponseBody(
std::ostream& _output, 
HttpResponse& _response, long _len) {
    long rest, cur;
    server_err_t error;
    HttpResponse::body_t body;

    error = SERVER_OK;
    body = _response.getBody(); body->seekg(0, body->beg); rest = _len;
    while (rest > 0) {
        if (!body->fail()) cur = body->readsome(buff, 4096);
        else cur = rest < 4096 ? rest : 4096;
        _output.write(buff, cur); rest -= cur;

        if (rest>0 && body->fail()) {
            std::memset(buff, 0, 4096);
            error = GEN_RESPONSE_BODY_FAILED;
        }
        if (_output.fail()) { error = GEN_RESPONSE_BODY_FAILED; break; }
    }
    return error;
}

// serialize HttpResponse to output, use a ResourceAccessor to access concrete response
server_err_t HttpResponsor::response(
std::ostream& _output, 
HttpResponse& _response) {
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
