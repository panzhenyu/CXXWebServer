#include <functional>
#include "Resource.hpp"
#include "../Error.hpp"
#include "../Cache/Cache.hpp"

class HttpRequest;
class HttpResponse;

struct IResourceAccessor {
    using reader_sptr_t = std::shared_ptr<std::istream>;
    using handler_t     = std::function<int(HttpRequest&, HttpResponse&)>;
protected:
    IResourceAccessor() = default;
    IResourceAccessor(const IResourceAccessor&) = delete;
    virtual ~IResourceAccessor() = default;
public:
    virtual reader_sptr_t access(const StaticResource&, server_err_t&) = 0;
    // virtual handler_t access(const CGIResource&, server_err_t&) = 0;
};

/*
 * ResourceAccessor
 * ResourceAccessor will be access by many workers, so it must enable thread-safe fucntions.
 * ResourceAccessor will set errno(in type server_err_t&) when the resource is not found.
 * When function access is invoked, it means the ValidReousrce(STATIC/CGI) is already in router.
 * The Accessor access all local resources in following 2 steps:
 * 1. Server Cache
 * 2. FileLoader.
 */
class CachedResourceAccessor: public IResourceAccessor {
    using cache_sptr_t = std::shared_ptr<ICache<std::string, std::string>>;
public:
    CachedResourceAccessor(cache_sptr_t);
    CachedResourceAccessor(const CachedResourceAccessor&) = delete;
    ~CachedResourceAccessor() = default;
    virtual reader_sptr_t access(const StaticResource&, server_err_t&) override;
    // virtual handler_t access(const CGIResource&, server_err_t&) override;
private:
    cache_sptr_t __cache;
};

/*
 * Local File Loader
 * Help to get a read only file handler(ifstream)
 */
class FileAccessor: public IResourceAccessor {
public:
    FileAccessor() = default;
    FileAccessor(const FileAccessor&) = delete;
    ~FileAccessor() = default;
    virtual reader_sptr_t access(const StaticResource&, server_err_t&) override;
    // virtual handler_t access(const CGIResource&, server_err_t&) override;
};
