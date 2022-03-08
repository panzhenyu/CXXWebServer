#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <atomic>
#include <memory>
#include <cstdint>
#include <unistd.h>
#include <netinet/in.h>
#include <unordered_map>
#include "Error.hpp"

#define DOMAIN      AF_INET
#define PROTO       IPPROTO_TCP
#define MAX_IPLEN   16
#define MAX_PASSWD  64

class Event;
class Worker;
class Router;
struct IResourceAccessor;

struct ServerConfig {
    using timeout_t = std::chrono::milliseconds;
    uint32_t    __maxFD;
    uint16_t    __clientTimeoutSecond;
    uint16_t    __workerNum;
    uint32_t    __serverAddr;
    uint16_t    __serverPort;
    uint16_t    __listenQueue;
    struct {
        char        __redisServerAddr[MAX_IPLEN];
        uint16_t    __redisServerPort;
        char        __redisServerPasswd[MAX_PASSWD];
        bool        __redisServerHasPasswd;
        uint8_t     __maxConn;
        unsigned    __maxRef;
        timeout_t   __responseTimeout;
        timeout_t   __lockTimeout;
    }           __redisConf;
    bool        __redisCacheEnable;
};

class Server {
public:
    using event_sptr_t      = std::shared_ptr<Event>;
    using worker_sptr_t     = std::shared_ptr<Worker>;
    using router_sptr_t     = std::shared_ptr<Router>;
    using accessor_sptr_t   = std::shared_ptr<IResourceAccessor>;
private:
    /* server config */
    std::string                 __cfgPath;
    ServerConfig                __serverCfg;
    /* server socket */
    int                         __listenFD;
    sockaddr_in                 __serverAddress;
    /* running status */
    std::atomic<bool>           __running;
    /* server managed resources */
    router_sptr_t               __router;
    accessor_sptr_t             __resourceAccessor;
    /* workers */
    std::vector<worker_sptr_t>  __ioWorkers;
    worker_sptr_t               __listenWorker;
private:
    /*
     * These initialize functions just init corresponding
     * reousrces, do not clean it even if error occurs.
     */
    server_err_t loadCfgFrom(std::string);
    server_err_t initListenSocket();
    server_err_t initRouter();
    server_err_t initResourceAccessor();
    server_err_t initListenWorker();
    server_err_t initIOWorker();
    server_err_t init();
    void clean();
public:
    Server();
    Server(const char* _cfgPath);
    Server(const Server&) = delete;
    ~Server();

    server_err_t start();
    void stop();

    bool isRunning();
    server_err_t statusChecking();
    server_err_t resume();
};
