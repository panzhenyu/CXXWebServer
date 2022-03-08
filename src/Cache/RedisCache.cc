#include <cstring>
#include "RedisCache.hpp"

/* RedisConnection member functions */

RedisConnection::RedisConnection(): 
__context(nullptr), __lockTimeout(0), __reqTimeout(DEFAULT_REPLY_TIMEOUT) {}

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
    bool fail;
    redisReply *reply;

    if (__context != nullptr) return REDIS_INIT_ALREADYDONE;
    std::lock_guard<std::timed_mutex> lck(__lock);
    // create redis context
    __context = redisConnectWithTimeout(__ip.c_str(), __port, {0, __reqTimeout.count()});
    if (__context == nullptr) return REDIS_CONN_TIMEOUT;
    if (__context->err) return REDIS_INIT_FAILED;

    // if set passwd
    if (__passwd.length()) {
        reply = (redisReply*)redisCommand(__context, "AUTH %s", __passwd.c_str());
        if (reply == nullptr) return REDIS_NULL_REPLY; 
        fail = reply->type==REDIS_REPLY_ERROR || (reply->type==REDIS_REPLY_STATUS && std::strcmp("OK", reply->str));
        freeReplyObject(reply);
        if (fail) return REDIS_PASSWD_WRONG;
    }

    // if set reply timeout
    redisSetTimeout(__context, {0, __reqTimeout.count()});
    return SERVER_OK;
}

bool RedisConnection::getKey(const key_t& _key, val_t& _retval) {
    bool ok;
    redisReply *reply;
    
    if (__context == nullptr) return false;
    if (__lockTimeout.count() == 0) __lock.lock();
    else if (!__lock.try_lock_for(__lockTimeout)) return false;
    
    reply = (redisReply*)redisCommand(__context, "GET %s", _key.c_str());
    if (reply == nullptr) return false;

    switch (reply->type) {
        case REDIS_REPLY_STRING:
            _retval = std::string(reply->str, reply->len); break;
        case REDIS_REPLY_INTEGER:
            _retval = std::to_string(reply->integer); break;
        default:
            ok = false; goto out;
    }
    ok = true;

out:
    freeReplyObject(reply);
    __lock.unlock();
    return ok;
}

bool RedisConnection::putKey(const key_t& _key, const val_t& _val) {
    bool ok;
    redisReply *reply;

    if (__context == nullptr) return false;
    if (__lockTimeout.count() == 0) __lock.lock();
    else if (!__lock.try_lock_for(__lockTimeout)) return false;

    reply = (redisReply*)redisCommand(__context, "SET %s %b", 
        _key.c_str(), _val.data(), _val.length());
    if (reply == nullptr) return false;
    if (reply->type==REDIS_REPLY_STATUS && 
        !std::strcmp("OK", reply->str)) ok = true;
    else ok = false;

    freeReplyObject(reply);
    __lock.unlock();
    return ok;
}

/* RedisConnectionBuilder */
RedisConnectionBuilder::RedisConnectionBuilder(): 
__obj(std::shared_ptr<RedisConnection>(new RedisConnection)) {}

IRedisConnectionBuilder& 
RedisConnectionBuilder::setIP(const std::string& _ip) {
    __obj->setIP(_ip); return *this;
}

IRedisConnectionBuilder& 
RedisConnectionBuilder::setPort(uint16_t _port) {
    __obj->setPort(_port); return *this;
}

IRedisConnectionBuilder& 
RedisConnectionBuilder::setPasswd(const std::string& _passwd) {
    __obj->setPasswd(_passwd); return *this;
}

IRedisConnectionBuilder& 
RedisConnectionBuilder::setLockTimeout(timeout_t _lt) {
    __obj->setLockTimeout(_lt); return *this;
}

IRedisConnectionBuilder& 
RedisConnectionBuilder::setReqTimeout(timeout_t _rt) {
    __obj->setReqTimeout(_rt); return *this;
}

IRedisConnectionBuilder& 
RedisConnectionBuilder::setPool(pool_sptr_t _pool) {
    __obj->setPool(_pool); return *this;
}

RedisConnectionBuilder::conn_sptr_t 
RedisConnectionBuilder::build() { return __obj; }

/* RedisConnectionPool member functions */
RedisConnectionPool::RedisConnectionPool(uint8_t _maxConn, unsigned _maxRef): 
__maxConn(_maxConn), __maxRef(_maxRef) {}

RedisConnectionPool::~RedisConnectionPool() { clear(); }

server_err_t RedisConnectionPool::pushConnection(const conn_sptr_t _conn) {
    if (nullptr == _conn) return REDIS_POOL_CONN_NULL;

    std::lock_guard<std::mutex> lck(__lock);
    if (__conns.size() >= __maxConn) return REDIS_POOL_MAXCONN;
    if (__conns.count(_conn)) return REDIS_POOL_CONNEXIST;
    __conns.emplace(_conn, 0);
    return SERVER_OK;
}

uint8_t RedisConnectionPool::curConnNum() {
    std::lock_guard<std::mutex> lck(__lock);
    return __conns.size();
}

server_err_t RedisConnectionPool::clear() {
    std::lock_guard<std::mutex> lck(__lock);
    __conns.clear();
    return SERVER_OK;
}

RedisConnectionPool::conn_sptr_t RedisConnectionPool::getOneConnection() {
    conn_sptr_t ret;
    ref_count_t minref;

    std::lock_guard<std::mutex> lck(__lock);
    minref = __maxRef + 1;
    for (auto& [conn, ref] : __conns) {
        if (ref < minref) {
            ret = conn;
            minref = ref;
        }
    }
    if (minref >= __maxRef) return nullptr;
    ++__conns[ret];

    return ret;
}

void RedisConnectionPool::returnConnection(const conn_sptr_t _conn) {
    if (nullptr == _conn) return;
    std::lock_guard<std::mutex> lck(__lock);
    if (__conns.count(_conn) && __conns[_conn]) --__conns[_conn];
}

/* RedisCache member functions */
RedisCache::RedisCache(poll_sptr_t _ppool): __connPool(_ppool) {}

bool RedisCache::get(const key_t& _key, val_t& _retval) {
    bool result;
    RedisConnectionPool::conn_sptr_t conn;

    if (_key.length() >= MAX_KEY_LEN) return false;
    if (nullptr == (conn=__connPool->getOneConnection())) return false;

    result = conn->getKey(_key, _retval);
    __connPool->returnConnection(conn);
    return result;
}

bool RedisCache::put(const key_t& _key, const val_t& _val) {
    bool result;
    RedisConnectionPool::conn_sptr_t conn;

    if (_key.length()>=MAX_KEY_LEN || _val.length()>=MAX_VAL_LEN) return false;
    if (nullptr == (conn=__connPool->getOneConnection())) return false;

    result = conn->putKey(_key, _val);
    __connPool->returnConnection(conn);
    return result;
}
