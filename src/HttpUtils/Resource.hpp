#pragma once

#include <mutex>
#include <string>

class HttpResponse;

struct Resource {
    enum ResourceType { STATIC, CGI, UNKNOWN };

    ResourceType    __type;
    std::string     __path;
    std::string     __uri;

    Resource();
    ~Resource() = default;
    ResourceType getType();
    std::string getURI();
    std::string getPath();
};

/*
 * ResourceAccessor
 * A singleton pattern, must achieve thread-safe public function
 * for static resource, return an fstream
 * for dynamic resource, just submit it to cgi process
 * ResourceAccessor will modify response object
 */
class ResourceAccessor {
public:
private:
    ResourceAccessor();
public:
    const ResourceAccessor& getResourceAccessor();

private:
    Resource&       __resource;
    HttpResponse&   __response;
    std::mutex      __lock;
};


