#pragma once

#include <string>
#include <unordered_map>
#include "Resource.hpp"

// singleton
class Router {
public:
    using uri_t = std::string;
    using resource_t = Resource;
    using mapper_t = std::unordered_map<uri_t, Resource>;
private:
    Router();
    int reload();
public:
    Router(std::string&) = delete;
    Router(const Router&) = delete;
    ~Router();
    Router& getRouter();
    void setRouterPath(std::string&);
    resource_t operator[](uri_t&);
private:
    static Router&  __router;
    std::string     __mappingFile;
    mapper_t        __resourceMapper;
};
