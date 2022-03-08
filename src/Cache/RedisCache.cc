#include "RedisCache.hpp"

/* RedisConnection member functions */

RedisConnection::RedisConnection(): __context(nullptr), __lockTimeout(-1), __reqTimeout(-1) {}

RedisConnection::~RedisConnection() {
    if (__context != nullptr) redisFree(__context);
    if (!__pool.expired()) __pool.reset();
}

void RedisConnection::setIP(const std::string& _ip) { __ip = _ip; }

void RedisConnection::setPort(uint16_t _port) { __port = _port; }

void RedisConnection::setPasswd(const std::string& _passwd) { __passwd = _passwd; }

void RedisConnection::setLockTimeout(timeout_t _lockTimeout) { __lockTimeout = _lockTimeout; }

void RedisConnection::setReqTimeout(timeout_t _reqTimeout) { __reqTimeout = _reqTimeout; }

void RedisConnection::setPool(pool_sptr_t _ppool) { __pool = _ppool; }

server_err_t RedisConnection::contextInit() {
    std::lock_guard<std::timed_mutex> lck(__lock);
    __context = redisConnectWithTimeout(__ip.c_str(), __port, {2, 0});
    if (__context==nullptr || __context->err) return REDIS_SERVER_INIT_FAILED;
}

bool RedisConnection::getKey(const key_t& _key, val_t& _retval) {

}

bool RedisConnection::putKey(const key_t& _key, const val_t& _val) {

}

/* RedisConnectionBuilder */
RedisConnectionBuilder::RedisConnectionBuilder(): 
__obj(std::shared_ptr<RedisConnection>(new RedisConnection)) {}

IRedisConnectionBuilder& RedisConnectionBuilder::setIP(const std::string& _ip) {
    __obj->setIP(_ip);
    return *this;
}

IRedisConnectionBuilder& RedisConnectionBuilder::setPort(uint16_t _port) {
    __obj->setPort(_port);
    return *this;
}

IRedisConnectionBuilder& RedisConnectionBuilder::setPasswd(const std::string& _passwd) {
    __obj->setPasswd(_passwd);
    return *this; 
}

IRedisConnectionBuilder& RedisConnectionBuilder::setLockTimeout(timeout_t _lt) {
    __obj->setLockTimeout(_lt);
    return *this;
}

IRedisConnectionBuilder& RedisConnectionBuilder::setReqTimeout(timeout_t _rt) {
    __obj->setReqTimeout(_rt);
    return *this;
}

IRedisConnectionBuilder& RedisConnectionBuilder::setPool(pool_sptr_t _pool) {
    __obj->setPool(_pool);
    return *this;
}

RedisConnectionBuilder::conn_sptr_t RedisConnectionBuilder::build() { return __obj; }

/* RedisConnectionPool member functions */
bool RedisConnectionPool::PrioQueueComp::operator()(
const RedisConnectionPool::prioq_elem_t& _chs, 
const RedisConnectionPool::prioq_elem_t& _rhs) const {
    return _chs.second > _rhs.second;
}

RedisConnectionPool::RedisConnectionPool(uint8_t _maxConn, unsigned _maxRef): 
__maxConn(_maxConn), __maxRef(_maxRef) {}

RedisConnectionPool::~RedisConnectionPool() { clear(); }

server_err_t RedisConnectionPool::pushConnection(const conn_sptr_t _conn) {
    if (nullptr == _conn) return NULL_REDIS_CONNECTION;
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

void RedisConnectionPool::returnConnection(const conn_sptr_t) {

}

/* RedisCache member functions */
RedisCache::RedisCache(poll_sptr_t _ppool): __connPool(_ppool) {}

bool RedisCache::get(const key_t& _key, val_t& _retval) {
    bool result;
    RedisConnectionPool::conn_sptr_t conn;

    conn = __connPool->getOneConnection();
    result = conn->getKey(_key, _retval);
    __connPool->returnConnection(conn);

    return true;
}

bool RedisCache::put(const key_t& _key, const val_t& _val) {
    bool result;
    RedisConnectionPool::conn_sptr_t conn;

    conn = __connPool->getOneConnection();
    result = conn->putKey(_key, _val);
    __connPool->returnConnection(conn);

    return result;
}
