#include <queue>
#include <hiredis/hiredis.h>
#include "Cache.hpp"
#include "../HttpUtils/ResourceAccessor.hpp"

class RedisConnectionPool;
/*
 * RedisConnection
 * 1. Should implement thread-safe functions.
 * 2. A RedisConnection will be assigned to some threads, 
 *    cause waiting lock for a while may be more efficient
 *    than read file.
 * 3. Use a time lock to avoid waiting for a long time.
 */
class RedisConnection {
public:
    using timeout_t = std::chrono::milliseconds;
public:
    RedisConnection(redisContext*, RedisConnectionPool&, timeout_t _lockTimeout, timeout_t _reqTimeout);
    RedisConnection(const RedisConnection&) = delete;
    ~RedisConnection();

private:
    redisContext*           __context;
    RedisConnectionPool&    __pool;
    std::timed_mutex        __lock;
    timeout_t               __lockTimeout;
    timeout_t               __reqTimeout;
};

/*
 * RedisConnectionPool
 * Must implement thread-safe member functions.
 */
class RedisConnectionPool {
    struct PrioQueueComp;
public:
    using ref_count_t       = unsigned;
    using conn_sptr_t       = std::shared_ptr<RedisConnection>;
    using prioq_elem_t      = std::pair<conn_sptr_t, ref_count_t>;
    using conn_prio_queue_t = std::priority_queue<prioq_elem_t, std::vector<prioq_elem_t>, PrioQueueComp>;
private:
    struct PrioQueueComp {
        // Build a small root heap to return the conn with minimum use count.
        bool operator()(const prioq_elem_t& _chs, const prioq_elem_t& _rhs) const;
    };
public:
    RedisConnectionPool(uint8_t, unsigned);
    RedisConnectionPool(const RedisConnectionPool&) = delete;
    ~RedisConnectionPool();

    server_err_t pushConnection(conn_sptr_t);
    uint8_t curConnNum();
    server_err_t clear();
    /*
     * Choose an RedisConnection from connection pool and return it.
     * If there isn't a RedisConnection available, return nullptr.
     */
    conn_sptr_t getOneConnection();
private:
    /*
     * This lock only protected __conns, cause __maxConn
     * and __maxRefPerConn won't be changed anymore.
     */
    std::mutex          __lock;     
    conn_prio_queue_t   __conns;
    uint8_t             __maxConn;
    unsigned            __maxRef;
};

/*
 * RedisCache
 * Use a thread-safe RedisConnectionPool
 * RedisCache must implement thread-safe member functions
 */
class RedisCache: public ICache<ValidResource::path_t, std::string> {
    using redis_pool_stpr_t = std::shared_ptr<RedisConnectionPool>;
public:
    RedisCache(redis_pool_stpr_t);
    RedisCache(const RedisCache&) = delete;
    virtual ~RedisCache() = default;
    // get a RedisConnection from RedisConnectionPool and do put|get with it.
    virtual std::pair<bool, val_t> get(const key_t&) override;
    virtual bool put(const key_t&, const val_t&) override;
private:
    redis_pool_stpr_t   __connPool;
};
