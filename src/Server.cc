#include <fcntl.h>
#include <cassert>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include "Error.hpp"
#include "Epoll.hpp"
#include "Event.hpp"
#include "Worker.hpp"
#include "Server.hpp"
#include "Log/Logger.hpp"
#include "IOEventHandler.hpp"
#include "Log/AsyncLogger.hpp"
#include "Cache/RedisCache.hpp"
#include "HttpUtils/Router.hpp"
#include "ListenEventHandler.hpp"

static ServerConfig defaultCfg = {
    .__maxFD = 100000, 
    .__clientTimeoutSecond = 60, 
    .__workerNum = 5, 
    .__serverAddr = INADDR_ANY, 
    .__serverPort = 8888, 
    .__listenQueue = 1000, 
    .__redisConf = {
        .__redisServerPort = 6379, 
        .__redisServerHasPasswd = true, 
        .__maxConn = 8, 
        .__maxRef = 2, 
        .__responseTimeout = ServerConfig::timeout_t(500), 
        .__lockTimeout = ServerConfig::timeout_t(1000), 
    }, 
    .__redisCacheEnable = true, 
};

server_err_t Server::loadCfgFrom(std::string _cfgPath) {
    // load config from path
    return SERVER_OK;
}

server_err_t Server::initListenSocket() {
    int flags;

    __serverAddress.sin_addr.s_addr = __serverCfg.__serverAddr;
    __serverAddress.sin_port = htons(__serverCfg.__serverPort);
    __serverAddress.sin_family = DOMAIN;
    if (-1 == (__listenFD=socket(DOMAIN, SOCK_STREAM, PROTO))) return SOCKET_INIT_FAILED;
    if (-1 == (flags=fcntl(__listenFD, F_GETFL, 0))) return SOCKET_INIT_FAILED;
    fcntl(__listenFD, F_SETFL, flags | O_NONBLOCK);
    if (-1 == bind(__listenFD, (sockaddr*)&__serverAddress, sizeof(sockaddr))) return SOCKET_BIND_FAILED;
    if (-1 == listen(__listenFD, __serverCfg.__listenQueue)) return SOCKET_LISTEN_FAILED;

    return SERVER_OK;
}

server_err_t Server::initRouter() {
    __router = std::make_shared<Router>();
    return SERVER_OK;
}

server_err_t Server::initResourceAccessor() {
    int i;
    server_err_t error;
    std::shared_ptr<RedisConnection> conn;
    std::shared_ptr<RedisCache> redisCache;
    std::shared_ptr<RedisConnectionPool> redisConnPool;
    auto redisConf = &__serverCfg.__redisConf;

    if (__serverCfg.__redisCacheEnable) {
        // init conn, connPool, cache, accessor
        redisConnPool = std::make_shared<RedisConnectionPool>(redisConf->__maxConn, redisConf->__maxRef);
        for (i=0; i<redisConf->__maxConn; ++i) {
            conn = RedisConnectionBuilder()
                .setIP(redisConf->__redisServerAddr)
                .setPort(redisConf->__redisServerPort)
                .setReqTimeout(redisConf->__responseTimeout)
                .setLockTimeout(redisConf->__lockTimeout)
                .setPasswd(redisConf->__redisServerHasPasswd ? redisConf->__redisServerPasswd : "")
                .build();
            if (SERVER_OK != (error=conn->contextInit())) return error;
            if (SERVER_OK != (error=redisConnPool->pushConnection(conn))) return error;
        }
        if (redisConnPool->curConnNum()) redisCache = std::make_shared<RedisCache>(redisConnPool);
        __resourceAccessor = std::make_shared<CachedResourceAccessor>(redisCache);
    } else __resourceAccessor = std::make_shared<FileAccessor>();
    return SERVER_OK;
}

server_err_t Server::initLogger() {
    auto& logger = AsyncLogger::getAsyncLogger();
    if (!logger.isRunning()) return LOG_THREAD_STOPPED;
    if (logger.logFileOn()) return SERVER_OK;
    return logger.setLogFile(DEFAULT_LOG);
}

server_err_t Server::initIOWorker() {
    int i;
    server_err_t error;
    worker_sptr_t worker;

    for (i=0; i<__serverCfg.__workerNum; ++i) {
        worker = std::make_shared<Worker>(
            std::make_unique<Epoll>(__serverCfg.__maxFD, error), 
            std::make_unique<IOEventHandler>(__router, __resourceAccessor));
        if (SERVER_OK != error) return error;
        __ioWorkers.push_back(worker);
    }

    return SERVER_OK;
}

server_err_t Server::initListenWorker() {
    server_err_t error;

    __listenWorker = std::make_shared<Worker>(
        std::make_unique<Epoll>(__serverCfg.__maxFD, error), 
        std::make_unique<ListenEventHandler>(__ioWorkers));
    if (SERVER_OK != error) return error;
    if (SERVER_OK != (error=__listenWorker->addEvent(
        std::make_shared<Event>(__listenFD, EPOLLIN|EPOLLET)))) return error;

    return SERVER_OK;
}

Server::Server(): Server(nullptr) {}

Server::Server(const char* _cfgPath):
__cfgPath(""), __listenFD(-1), __running(false) {
    if (_cfgPath) { __cfgPath = _cfgPath; loadCfgFrom(_cfgPath); }
    else {
        std::strcpy(defaultCfg.__redisConf.__redisServerAddr, "127.0.0.1");
        std::strcpy(defaultCfg.__redisConf.__redisServerPasswd, "panda");
        defaultCfg.__redisConf.__redisServerHasPasswd = true;
        __serverCfg = defaultCfg;
    }
    __listenFD = -1;
}

Server::~Server() {
    stop();
    clean();
    LOG2DIARY << "server out" << '\n';
}

/*
 * Initialize resources for server.
 * Cannot change the order of initialization.
 */
server_err_t Server::init() {
    server_err_t error;

    clean();
    if (SERVER_OK != (error=initLogger())) return error;
    if (SERVER_OK != (error=initRouter())) return error;
    if (SERVER_OK != (error=initResourceAccessor())) return error;
    if (SERVER_OK != (error=initIOWorker())) return error;
    if (SERVER_OK != (error=initListenSocket())) return error;
    if (SERVER_OK != (error=initListenWorker())) return error;
    return SERVER_OK;
}

/* start worker threads */
server_err_t Server::start() {
    server_err_t error;

    if (isRunning()) return SERVER_ALREADY_RUNNING;
    if (SERVER_OK != (error=init())) return error;

    for (auto& ioWorker : __ioWorkers) {
        if (SERVER_OK != (error=ioWorker->run())) goto stop;
    }
    if (SERVER_OK != (error=__listenWorker->run())) goto stop;
    __running = true;
    goto out;

stop:
    stop();
    clean();
out:
    return error;
}

void Server::stop() {
    for (auto& ioWorker : __ioWorkers) if (ioWorker && ioWorker->isRunning()) ioWorker->stop();
    if (__listenWorker && __listenWorker->isRunning()) __listenWorker->stop();
    __running = false;
}

/*
 * Clean all resources, this function should be 
 * careful with those not initialized.
 */
void Server::clean() {
    if (-1 == __listenFD) close(__listenFD);
    __router.reset();
    __resourceAccessor.reset();
    __listenWorker.reset();
    __ioWorkers.clear();
}

bool Server::isRunning() { return __running; }

server_err_t Server::statusChecking() {
    if (!__listenWorker->isRunning()) return LISTEN_WORKER_DUMP;
    for (auto& ioWorker : __ioWorkers) {
        if (!ioWorker->isRunning()) return IO_WORKER_DUMP;
    }
    return SERVER_OK;
}
