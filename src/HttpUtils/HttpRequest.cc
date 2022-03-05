#include <algorithm>
#include "HttpRequest.hpp"

HttpRequest::HttpRequest(): 
    __method(HttpRequestMethod::UNKNOWN_REQUEST_METHOD), 
    __version(HttpVersion::UNKNOWN_HTTP_VERSION) {}

HttpRequestMethod HttpRequest::getRequestMethod() { return __method; }

HttpRequest::uri_t& HttpRequest::getURI() { return __uri; }

HttpVersion HttpRequest::getHttpVersion() { return __version; }

HttpRequest::header_t& HttpRequest::getHeader() { return __header; }

HttpRequest::body_t& HttpRequest::getBody() { return __body; }

void HttpRequest::setRequestMethod(HttpRequestMethod _m) { __method = _m; }

void HttpRequest::setHttpVersion(HttpVersion _v) { __version = _v; }

void HttpRequest::setURI(const uri_t& _uri) { __uri = _uri; }

void HttpRequest::setHeader(const key_t& _k, const value_t& _v) { __header[_k] = _v; }

void HttpRequest::delHeader(const key_t& _k) {
    if (__header.count(_k)) __header.erase(_k);
}

void HttpRequest::setBody(const body_t& _body) { __body = _body; }

// std::string HttpRequest::serialize() {
//     return "";
// }

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

bool HttpRequestAnalyser::haveRequestBody(request_sptr_t _req) {
    return _req->getHeader().count("content-length") > 0;
}

server_err_t HttpRequestAnalyser::skipTerminateCH() {
    if ('\r'!=__input->get() || __input->fail() || '\n'!=__input->get() || __input->fail())
        return PARSE_TERMINATE_CH_FAILED;
    return SERVER_OK;
}

/*
 * Analyse request line for HttpRequest
 * error only occurs when it failed to get string from input
 */
server_err_t HttpRequestAnalyser::parseLine(HttpRequestBuilder& _builder) {
    std::string method, uri, version;

    if ((*__input>>method).fail()) return PARSE_REQ_METHOD_FAILED;
    if ((*__input>>uri).fail()) return PARSE_REQ_URI_FAILED;
    if ((*__input>>version).fail()) return PARSE_REQ_VERSION_FAILED;
    _builder.setRequestMethod(toRequestMethod(method))
            .setURI(uri)
            .setHttpVersion(toHttpVersion(version));

    return skipTerminateCH();
}

/*
 * Parse Headers for HTTP request
 * All key will be reserved in lowercase
 */
server_err_t HttpRequestAnalyser::parseHead(HttpRequestBuilder& _builder) {
    size_t tok, slen;
    std::string cur, key, value;
    do {
        getline(*__input, cur, '\n');
        if (__input->fail()) return PARSE_REQ_HEADER_FAILED;
        if (std::string::npos == (tok=cur.find(':'))) return INVALID_REQ_HEADER_FORMAT;
        // stride \r and get key-value
        cur.pop_back(); slen = cur.length(); key = cur.substr(0, tok++);
        while (tok<slen && cur[tok]==' ') { ++tok; } value = cur.substr(tok);
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        _builder.setHeader(key, value);
    } while (*__input);
    return skipTerminateCH();
}

server_err_t HttpRequestAnalyser::parseBody(HttpRequestBuilder& _builder) {
    std::string body;
    if (!haveRequestBody(_builder.build())) return SERVER_OK;
    getline(*__input, body, '\n');
    if (__input->fail()) return PARSE_REQ_BODY_FAILED;
    body.pop_back();
    _builder.setBody(body);
    return SERVER_OK;
}

HttpRequestAnalyser::HttpRequestAnalyser(input_uptr_t&& _input): __input(std::move(_input)) {}

HttpRequestAnalyser::request_sptr_t HttpRequestAnalyser::getOneHttpRequest(server_err_t& _err) {
    HttpRequestBuilder reqBuilder;
    if (SERVER_OK != (_err=parseLine(reqBuilder))) goto out;
    if (SERVER_OK != (_err=parseHead(reqBuilder))) goto out;
    if (SERVER_OK != (_err=parseBody(reqBuilder))) goto out;
out:
    return reqBuilder.build();
}

// std::ostream operator<<(std::ostream& _out, HttpRequest& _req) { }
