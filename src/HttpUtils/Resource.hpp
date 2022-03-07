#pragma once

#include <mutex>
#include <memory>
#include <string>
#include "HttpConfig.hpp"

class GeneralResource {
public:
    enum ResourceType { STATIC, CGI, INVALID };
protected:
    GeneralResource(ResourceType);
    GeneralResource(const GeneralResource&) = default;
    ~GeneralResource() = default;
public:
    ResourceType getType() const;
private:
    ResourceType __type;
};

class InvalidResource: public GeneralResource {
public:
    InvalidResource();
    InvalidResource(const InvalidResource&) = default;
    ~InvalidResource() = default;
};

class ValidResource: public GeneralResource {
public:
    using path_t    = std::string;
    using uri_t     = std::string;
protected:
    ValidResource(ResourceType, const path_t&, const uri_t&);
public:
    ValidResource(const ValidResource&) = default;
    std::string getPath() const;
    std::string getURI() const;
private:
    std::string __path;
    std::string __uri;
};

class StaticResource: public ValidResource {
public:
    StaticResource(const path_t&, const uri_t&);
    StaticResource(const StaticResource&) = default;
    ~StaticResource() = default;
};

struct CGIResource: public ValidResource {
public:
    CGIResource(const path_t&, const uri_t&);
    CGIResource(const CGIResource&) = default;
    ~CGIResource() = default;
};
