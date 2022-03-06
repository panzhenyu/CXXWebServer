#include "ResourceAccessor.hpp"

// set state code and errno if error occurs
LocalResourceAccessor::reader_sptr_t LocalResourceAccessor::access(
const StaticResource& _sr,
server_err_t& _error) {
    std::string path = _sr.getPath();

    _error = STATIC_RESOURCE_ACCESS_FAILED;
    return nullptr;
}
