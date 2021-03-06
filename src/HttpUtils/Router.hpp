#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include "Resource.hpp"
#include "../Error.hpp"
#include "HttpConfig.hpp"

#define DEFAULT_MAPPING_PATH    "web.xml"

struct IRouter {
public:
    using uri_t             = std::string;
    using gresource_sptr_t  = std::shared_ptr<GeneralResource>;
    using default_errpg_t   = std::shared_ptr<std::istringstream>;
protected:
    IRouter() = default;
    virtual ~IRouter() = default;
public:
    virtual server_err_t loadFrom(const std::string&) = 0;
    virtual gresource_sptr_t operator[](const uri_t&) = 0;
    virtual gresource_sptr_t operator[](statecode_t) = 0;
    virtual default_errpg_t getDefaultErrorPage(statecode_t) const = 0;
};

/* 
 * Router
 * Shared among many workers, the public member should guarantee thread-safe.
 * Router do not access resource, it just tell you how to access resource.
 */
class Router: public IRouter {
public:
    using vresource_sptr_t  = std::shared_ptr<ValidResource>;
    using uri_mapper = std::unordered_map<uri_t, vresource_sptr_t>;
    using err_mapper = std::unordered_map<statecode_t, vresource_sptr_t>;
private:
    server_err_t doLoad(const std::string&);
public:
    Router();
    ~Router() = default;
    virtual server_err_t loadFrom(const std::string&);
    virtual gresource_sptr_t operator[](const uri_t&);
    virtual gresource_sptr_t operator[](statecode_t);
    virtual default_errpg_t getDefaultErrorPage(statecode_t) const;
private:
    std::mutex      __lock;
    std::string     __mappingFilePath;
    uri_mapper      __uriMapper;
    err_mapper      __errMapper;
};
