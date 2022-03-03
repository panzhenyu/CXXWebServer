#pragma once

#include <string>
#include <unordered_map>
#include "Resource.hpp"

class Router {
public:
    using uri_t = std::string;
    using resource_t = Resource;
    using mapper_t = std::unordered_map<uri_t, Resource>;
public:
    Router(std::string&);
    Router(const Router&) = delete;
    ~Router();
    int reload();
    resource_t operator[](uri_t&);
private:
    std::string     __routeFile;
    mapper_t        __resourceMapper;
};
