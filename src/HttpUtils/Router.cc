#include "Router.hpp"

server_err_t ResourceMapper::load(std::string& _path) {
    return SERVER_OK;
}

ResourceMapper::resource_t ResourceMapper::operator[](uri_t& _uri) {
    return __uriResourceMap.count(_uri) ? __uriResourceMap[_uri] : Resource();
}

Router::Router() { loadFrom(DEFAULT_MAPPING_PATH); }

inline const Router& Router::getRouter() {
    static Router router;
    return router;
}

server_err_t Router::loadFrom(std::string&& _path_rval) { return loadFrom(_path_rval); }

server_err_t Router::loadFrom(std::string& _path) {
    server_err_t error;
    std::string oldPath;
    Router& router = const_cast<Router&>(Router::getRouter());
    mapper_t& mapper = router.__resourceMapper;

    std::lock_guard<std::mutex> lck(router.__lock);
    if (SERVER_OK == (error=mapper.load(_path))) router.__mappingFilePath = _path;
    return error;
}

Router::resource_t Router::operator[](uri_t&& _uri_rval) const { return (*this)[_uri_rval]; }

Router::resource_t Router::operator[](uri_t& _uri) const {
    Router& router = const_cast<Router&>(Router::getRouter());
    mapper_t& mapper = router.__resourceMapper;

    std::lock_guard<std::mutex> lck(router.__lock);
    return mapper[_uri];
}
