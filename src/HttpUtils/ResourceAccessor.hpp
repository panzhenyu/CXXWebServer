#include <functional>
#include "Resource.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

/*
 * StaticResourceAccessor
 * Singleton pattern, so it must achieve thread-safe public function
 * just handle local resource, return an fstream
 */
class StaticResourceAccessor {
public:
    using reader_sptr_t = std::shared_ptr<std::istream>;
protected:
    StaticResourceAccessor() = default;
public:
    static StaticResourceAccessor& getStaticResourceAccessor();
public:
    ~StaticResourceAccessor() = default;
    reader_sptr_t access(const StaticResource&, statecode_t&, server_err_t&);
private:
    std::mutex __lock;
};

class CGIResourceAccessor {
public:
    using handler_t = std::function<int(HttpRequest&, HttpResponse&)>;
public:
    handler_t access(const CGIResource&, server_err_t&);
private:
    std::mutex __lock;
};
