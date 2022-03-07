#include "RedisCache.hpp"

/* RedisConnection member functions */
RedisConnection::RedisConnection(
redisContext* _context, RedisConnectionPool& _pool, 
timeout_t _lockTimeout, timeout_t _reqTimeout): 
__context(_context), __pool(_pool), __lockTimeout(_lockTimeout), __reqTimeout(_reqTimeout) {}

RedisConnection::~RedisConnection() {
    redisFree(__context);
}

/* RedisConnectionPool member functions */
bool RedisConnectionPool::PrioQueueComp::operator()(
const RedisConnectionPool::prioq_elem_t& _chs, 
const RedisConnectionPool::prioq_elem_t& _rhs) const {
    return _chs.second > _rhs.second;
}

RedisConnectionPool::RedisConnectionPool(uint8_t _maxConn, unsigned _maxRef): 
__maxConn(_maxConn), __maxRef(_maxRef) {}

RedisConnectionPool::~RedisConnectionPool() { clear(); }

server_err_t RedisConnectionPool::pushConnection(conn_sptr_t _conn) {
    if (nullptr == _conn) return INVLIAD_REDIS_CONNECTION;
    std::lock_guard<std::mutex> lck(__lock);
    __conns.emplace(_conn, 0);
    return SERVER_OK;
}

uint8_t RedisConnectionPool::curConnNum() {
    std::lock_guard<std::mutex> lck(__lock);
    return __conns.size();
}

server_err_t RedisConnectionPool::clear() {
    std::lock_guard<std::mutex> lck(__lock);
    while (!__conns.empty()) __conns.pop();
    return SERVER_OK;
}

RedisConnectionPool::conn_sptr_t RedisConnectionPool::getOneConnection() {
    std::lock_guard<std::mutex> lck(__lock);
    if (__conns.empty()) return nullptr;
    if (__conns.top().second >= __maxRef) return nullptr;

    auto& [conn, refs] = __conns.top();
    __conns.pop(); __conns.emplace(conn, refs+1);
    return conn;
}

/* RedisCache member functions */
RedisCache::RedisCache(redis_pool_stpr_t) {

}

// std::pair<bool, RedisCache::val_t> RedisCache::get(const key_t&) {

// }

// bool RedisCache::put(const key_t&, const val_t&) {
//     return true;
// }