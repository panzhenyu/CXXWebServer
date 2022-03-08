#include <queue>
#include <chrono>
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
    friend class RedisConnectionBuilder;
public:
    using key_t         = std::string;
    using val_t         = std::string;
    using timeout_t     = std::chrono::milliseconds;
    using pool_wptr_t   = std::weak_ptr<RedisConnectionPool>;
    using pool_sptr_t   = std::weak_ptr<RedisConnectionPool>;
protected:
    /* Helper functions for RedisConnection initialization */
    RedisConnection();
    void setIP(const std::string&);
    void setPort(uint16_t);
    void setPasswd(const std::string&);
    void setLockTimeout(timeout_t);
    void setReqTimeout(timeout_t);
    void setPool(pool_sptr_t);
public:
    RedisConnection(const RedisConnection&) = delete;
    ~RedisConnection();

    bool getKey(const key_t&, val_t&);
    bool putKey(const key_t&, const val_t&);
    server_err_t contextInit();
    void close();
private:
    std::string         __ip;
    uint16_t            __port;
    std::string         __passwd;
    redisContext*       __context;
    pool_wptr_t         __pool;
    std::timed_mutex    __lock;
    timeout_t           __lockTimeout;
    timeout_t           __reqTimeout;
};

struct IRedisConnectionBuilder {
    using conn_sptr_t   = std::shared_ptr<RedisConnection>;
    using timeout_t     = RedisConnection::timeout_t;
    using pool_sptr_t   = RedisConnection::pool_sptr_t;
protected:
    IRedisConnectionBuilder() = default;
    IRedisConnectionBuilder(const IRedisConnectionBuilder&) = delete;
    virtual ~IRedisConnectionBuilder() = default;
public:
    virtual IRedisConnectionBuilder& setIP(const std::string&) = 0;
    virtual IRedisConnectionBuilder& setPort(uint16_t) = 0;
    virtual IRedisConnectionBuilder& setPasswd(const std::string&) = 0;
    virtual IRedisConnectionBuilder& setLockTimeout(timeout_t) = 0;
    virtual IRedisConnectionBuilder& setReqTimeout(timeout_t) = 0;
    virtual IRedisConnectionBuilder& setPool(pool_sptr_t) = 0;
    virtual conn_sptr_t build() = 0;
};

class RedisConnectionBuilder: public IRedisConnectionBuilder {
public:
    RedisConnectionBuilder();
    RedisConnectionBuilder(const RedisConnectionBuilder&) = delete;
    virtual ~RedisConnectionBuilder() = default;

    virtual IRedisConnectionBuilder& setIP(const std::string&);
    virtual IRedisConnectionBuilder& setPort(uint16_t);
    virtual IRedisConnectionBuilder& setPasswd(const std::string&);
    virtual IRedisConnectionBuilder& setLockTimeout(timeout_t);
    virtual IRedisConnectionBuilder& setReqTimeout(timeout_t);
    virtual IRedisConnectionBuilder& setPool(pool_sptr_t);
    virtual conn_sptr_t build();
private:
    conn_sptr_t __obj;
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

    server_err_t pushConnection(const conn_sptr_t);
    uint8_t curConnNum();
    server_err_t clear();
    /*
     * Choose an RedisConnection from connection pool and return it.
     * If there isn't a RedisConnection available, return nullptr.
     */
    conn_sptr_t getOneConnection();
    void returnConnection(const conn_sptr_t);
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
    using poll_sptr_t = std::shared_ptr<RedisConnectionPool>;
    static constexpr unsigned MAX_KEY_LEN = (1 << 20);
    static constexpr unsigned MAX_VAL_LEN = (1 << 20);
public:
    RedisCache(poll_sptr_t);
    RedisCache(const RedisCache&) = delete;
    virtual ~RedisCache() = default;
    // get a RedisConnection from RedisConnectionPool and do put|get with it.
    virtual bool get(const key_t&, val_t&) override;
    virtual bool put(const key_t&, const val_t&) override;
private:
    poll_sptr_t __connPool;
};
