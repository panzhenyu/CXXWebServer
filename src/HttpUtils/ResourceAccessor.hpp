#include <functional>
#include "Resource.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

struct LocalResourceAccessor {
    using reader_sptr_t = std::shared_ptr<std::istream>;
    using handler_t     = std::function<int(HttpRequest&, HttpResponse&)>;

    static reader_sptr_t access(const StaticResource&, server_err_t&);
    static handler_t access(const CGIResource&, server_err_t&);
};

/*
 * Local File Loader
 * Help to get a read only file handler(ifstream)
 */
class FileLoader {

public:
    
};
