#include <algorithm>
#include "HttpRequest.hpp"

#define BFSIZE  512
static char _local_static_buff[BFSIZE];

HttpRequest::HttpRequest(): 
    __method(HttpRequestMethod::UNKNOWN_REQUEST_METHOD), 
    __version(HttpVersion::UNKNOWN_HTTP_VERSION) {}

HttpRequestMethod HttpRequest::getRequestMethod() { return __method; }

const HttpRequest::uri_t& HttpRequest::getURI() const { return __uri; }

HttpVersion HttpRequest::getHttpVersion() const { return __version; }

const HttpRequest::header_t& HttpRequest::getHeader() const { return __header; }

const HttpRequest::body_t& HttpRequest::getBody() const { return __body; }

void HttpRequest::setRequestMethod(HttpRequestMethod _m) { __method = _m; }

void HttpRequest::setHttpVersion(HttpVersion _v) { __version = _v; }

void HttpRequest::setURI(const uri_t& _uri) { __uri = _uri; }

void HttpRequest::setHeader(const key_t& _k, const value_t& _v) { __header[_k] = _v; }

void HttpRequest::delHeader(const key_t& _k) {
    if (__header.count(_k)) __header.erase(_k);
}

void HttpRequest::setBody(const body_t& _body) { __body = _body; }

std::string HttpRequest::serialize() const {
    std::string ret = ::serialize(__method) + ' '
        + std::string(__uri) + ' '
        + ::serialize(__version) + '\n';
    for (auto& [key, value] : __header) {
        ret += key + ": " + value + '\n';
    }
    ret += '\n' + __body;
    return ret;
}

HttpRequestBuilder::HttpRequestBuilder(): 
    __obj(std::shared_ptr<HttpRequest>(new HttpRequest())) {}

HttpRequestBuilder::request_sptr_t HttpRequestBuilder::build() { return __obj; }

IHttpReqeustBuilder& HttpRequestBuilder::setRequestMethod(HttpRequestMethod _m) {
    __obj->setRequestMethod(_m);
    return *this;
}

IHttpReqeustBuilder& HttpRequestBuilder::setHttpVersion(HttpVersion _v) {
    __obj->setHttpVersion(_v);
    return *this;
}

IHttpReqeustBuilder& HttpRequestBuilder::setURI(HttpRequest::uri_t& _uri) {
    __obj->setURI(_uri);
    return *this;
}

IHttpReqeustBuilder& HttpRequestBuilder::setHeader(HttpRequest::key_t& _k, HttpRequest::value_t& _v) {
    __obj->setHeader(_k, _v);
    return *this;
}

IHttpReqeustBuilder& HttpRequestBuilder::delHeader(HttpRequest::key_t& _k) {
    __obj->delHeader(_k);
    return *this;
}

IHttpReqeustBuilder& HttpRequestBuilder::setBody(HttpRequest::body_t& _body) {
    __obj->setBody(_body);
    return *this;
}

size_t HttpRequestAnalyser::requestBodyLength(request_sptr_t _req) {
    if (_req->getHeader().count("Content-Length") == 0) return 0;
    return std::stoul(_req->getHeader().at("Content-Length"));
}

server_err_t HttpRequestAnalyser::skipTerminateCH(std::istream& _input) {
    if ('\r'!=_input.get() || _input.fail() || '\n'!=_input.get() || _input.fail())
        return PARSE_TERMINATE_CH_FAILED;
    return SERVER_OK;
}

/*
 * Analyse request line for HttpRequest
 * error only occurs when it failed to get string from input
 */
server_err_t HttpRequestAnalyser::parseLine(std::istream& _input, HttpRequestBuilder& _builder) {
    std::string method, uri, version;

    if ((_input>>method).fail()) return PARSE_REQ_METHOD_FAILED;
    if ((_input>>uri).fail()) return PARSE_REQ_URI_FAILED;
    if ((_input>>version).fail()) return PARSE_REQ_VERSION_FAILED;

    _builder.setRequestMethod(toRequestMethod(method))
            .setURI(uri)
            .setHttpVersion(toHttpVersion(version));
    return skipTerminateCH(_input);
}

/*
 * Parse Headers for HTTP request
 */
server_err_t HttpRequestAnalyser::parseHead(std::istream& _input, HttpRequestBuilder& _builder) {
    size_t tok, slen;
    std::string cur, key, value;
    do {
        getline(_input, cur, '\n');
        if (_input.fail()) return PARSE_REQ_HEADER_FAILED;
        if (cur.length() == 1) break;
        if (std::string::npos == (tok=cur.find(':'))) return INVALID_REQ_HEADER_FORMAT;
        // stride \r and get key-value
        cur.pop_back(); slen = cur.length(); key = cur.substr(0, tok++);
        while (tok<slen && cur[tok]==' ') { ++tok; } value = cur.substr(tok);
        _builder.setHeader(key, value);
    } while (_input);
    return SERVER_OK;
}

server_err_t HttpRequestAnalyser::parseBody(std::istream& _input, HttpRequestBuilder& _builder) {
    size_t bodyLen, rest, cur;
    std::string body;
    if (0 == (bodyLen=requestBodyLength(_builder.build()))) return SERVER_OK;

    rest = bodyLen;
    while (rest) {
        cur = rest < BFSIZE ? rest : BFSIZE;
        cur = _input.readsome(_local_static_buff, cur);
        body += std::string(_local_static_buff, cur);
        if (_input.fail()) return PARSE_REQ_BODY_FAILED;
        rest -= cur;
    }
    _builder.setBody(body);
    return SERVER_OK;
}

HttpRequestAnalyser::request_sptr_t HttpRequestAnalyser::getOneHttpRequest(
std::istream& _input, 
server_err_t& _err) {
    HttpRequestBuilder reqBuilder;
    if (SERVER_OK != (_err=parseLine(_input, reqBuilder))) goto out;
    if (SERVER_OK != (_err=parseHead(_input, reqBuilder))) goto out;
    if (SERVER_OK != (_err=parseBody(_input, reqBuilder))) goto out;
out:
    return reqBuilder.build();
}

std::ostream& operator<<(std::ostream& _out, const HttpRequest& _req) { return _out << _req.serialize(); }
