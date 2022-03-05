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
    ResourceType getType();
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
protected:
    ValidResource(ResourceType, const std::string&, const std::string&);
public:
    ValidResource(const ValidResource&) = default;
    std::string getPath();
    std::string getURI();
private:
    std::string __path;
    std::string __uri;
};

class StaticResource: public ValidResource {
public:
    StaticResource(const std::string&, const std::string&);
    StaticResource(const StaticResource&) = default;
    ~StaticResource() = default;
};

struct CGIResource: public ValidResource {
public:
    CGIResource(const std::string&, const std::string&);
    CGIResource(const CGIResource&) = default;
    ~CGIResource() = default;
};
