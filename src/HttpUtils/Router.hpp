#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include "Resource.hpp"
#include "../Error.hpp"

#define DEFAULT_MAPPING_PATH    "web.xml"
#define DEFAULT_ERROR_ROOT      "/err"

class ResourceMapper {
public:
    using uri_t         = std::string;
    using resource_t    = Resource;
public:
    ResourceMapper() = default;
    ResourceMapper(const ResourceMapper&) = delete;
    ~ResourceMapper();
    server_err_t load(std::string&);
    resource_t operator[](uri_t&);
private:
    std::unordered_map<uri_t, resource_t> __uriResourceMap;
};

// singleton, public member should guarantee thread-safe
// Router do not access resource, it just tell you how to access resource
// ResourceAccessor will deal with concrete resource and revise HttpResponse Object
class Router {
public:
    using mapper_t      = ResourceMapper;
    using uri_t         = mapper_t::uri_t;
    using resource_t    = mapper_t::resource_t;
private:
    Router();
public:
    static const Router& getRouter();
    static server_err_t loadFrom(std::string&&);
    static server_err_t loadFrom(std::string&);
public:
    Router(const Router&) = delete;
    ~Router() = default;
    resource_t operator[](uri_t&&) const;
    resource_t operator[](uri_t&) const;
private:
    std::mutex      __lock;
    std::string     __mappingFilePath;
    mapper_t        __resourceMapper;
};
