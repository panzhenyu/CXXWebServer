#include <fstream>
#include <sstream>
#include <cassert>
#include "ResourceAccessor.hpp"

CachedResourceAccessor::CachedResourceAccessor(cache_sptr_t _cache): __cache(_cache) {}

/*
 * Set state code 500 if neither the resource exist in cache nor in local file system.
 * Should be careful with following situations:
 * 1. The cache may not initialized correctly, check the cache_sptr_t.
 * 2. When cache miss occurs, use FileAccessor to find istream.
 * 3. Pay attention to the file size when you want to cache new resource in redis.
 * 4. The FileAccessor may also return a nullptr, this means the path has been mapped,
 *    but it doesn't exist anymore, then you should set a server internal error.
 */
CachedResourceAccessor::reader_sptr_t
CachedResourceAccessor::access(
const StaticResource& _sr, server_err_t& _error) {
    reader_sptr_t in;
    unsigned long filesize, rest, cur;
    std::string path, resource;
    
    in = nullptr; _error = SERVER_OK; path = _sr.getPath();
    // 1. Lookup cache first
    if (__cache!=nullptr && __cache->get(path, resource)) {
        in = std::make_shared<std::istringstream>(resource);
    } else {
        // 2. Lookup local by FileAccessor
        if (nullptr!=(in=FileAccessor().access(_sr, _error)) && _error==SERVER_OK && __cache!=nullptr) {
            // 3. Put data into cache if cache exists and resource is valid
            in->seekg(0, in->end); filesize = in->tellg(); in->seekg(0, in->beg); resource.resize(filesize);
            if (__cache->validKey(path) && __cache->validVal(resource)) {
                rest = filesize;
                while (rest > 0) {
                    cur = in->readsome(&resource.front()+(filesize-rest), rest);
                    rest -= cur;
                    if (in->fail()) { in.reset(); _error = FILE_READ_ABORT; return nullptr; }
                }
                __cache->put(path, resource);
                in = std::make_shared<std::istringstream>(resource);
            }
        }
    }
    return in;
}

/*
 * FileAccessor::load, load local file in read-only mode
 */
FileAccessor::reader_sptr_t
FileAccessor::access(const StaticResource& _sr, server_err_t& _err) {
    std::shared_ptr<std::ifstream> in;
    
    in = std::make_shared<std::ifstream>(_sr.getPath(), std::ios_base::binary | std::ios_base::in);

    if (in->is_open()) _err = SERVER_OK;
    else { _err = INVALID_FILE_PATH; in.reset(); in = nullptr; }

    return in;
}
