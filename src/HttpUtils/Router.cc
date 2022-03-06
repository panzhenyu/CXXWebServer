#include <sstream>
#include "Router.hpp"

Router::Router() { doLoad(DEFAULT_MAPPING_PATH); }

Router& Router::getRouter() {
    static Router router;
    return router;
}

server_err_t Router::doLoad(const std::string& _path) {
    // fill uri mapper & err mapper
    __uriMapper["/hello"] = std::make_shared<StaticResource>("./hello.html", "/hello");
    __errMapper[404] = std::make_shared<StaticResource>("./404.html", "/404");
    return SERVER_OK;
}

server_err_t Router::loadFrom(const std::string& _path) {
    server_err_t error;
    std::string oldPath;
    Router& router = const_cast<Router&>(Router::getRouter());

    std::lock_guard<std::mutex> lck(router.__lock);
    if (SERVER_OK == (error=doLoad(_path))) router.__mappingFilePath = _path;
    return error;
}

Router::gresource_sptr_t Router::operator[](statecode_t _code) {
    Router& router = Router::getRouter();
    err_mapper& mapper = router.__errMapper;

    std::lock_guard<std::mutex> lck(router.__lock);
    if (mapper.count(_code)) return std::make_shared<ValidResource>(*mapper.at(_code));
    return std::make_shared<InvalidResource>();
}

Router::gresource_sptr_t Router::operator[](const uri_t& _uri) {
    Router& router = Router::getRouter();
    uri_mapper& mapper = router.__uriMapper;

    std::lock_guard<std::mutex> lck(router.__lock);
    if (mapper.count(_uri)) return std::make_shared<ValidResource>(*mapper.at(_uri));
    return std::make_shared<InvalidResource>();
}

// This function doesn't need lock, cause all variable are function local
Router::default_errpg_t Router::getDefaultErrorPage(statecode_t _code) const {
    HttpResponseStatus status(_code);
    std::string pageInfo, codeInfo;

    codeInfo = std::to_string(_code) + " " + status.getDetail();
    pageInfo += "<html><head><title>" + codeInfo;
    pageInfo += "</title></head><body><h1>" + codeInfo;
    pageInfo += "</h1></body></html>";

    return std::make_shared<std::istringstream>(pageInfo);
}
