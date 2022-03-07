#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "Error.hpp"
#include "Epoll.hpp"
#include "Event.hpp"
#include "Worker.hpp"
#include "Server.hpp"
#include "IOEventHandler.hpp"
#include "ListenEventHandler.hpp"

static ServerConfig defaultCfg = {
    .__maxFD = 100000, 
    .__clientTimeoutSecond = 60, 
    .__workerNum = 7, 
    .__serverAddr = INADDR_ANY, 
    .__serverPort = 8888, 
    .__listenQueue = 1000, 
    .__redisConf = {
        // .__redisServerAddr = "127.0.0.1", 
        .__redisServerPort = 6379, 
        // .__redisServerPasswd = "panda", 
        .__redisServerEnable = true, 
        .__redisServerHasPasswd = true, 
        .__maxConn = 8, 
        .__maxRef = 2, 
        .__responseTimeout = ServerConfig::timeout_t(100), 
        .__lockTimeout = ServerConfig::timeout_t(800), 
    }, 
    .__cacheEnable = true, 
};

server_err_t Server::loadCfgFrom(std::string _cfgPath) {
    // load config from path
    return SERVER_OK;
}

Server::Server(): Server(nullptr) {}

Server::Server(const char* _cfgPath):
__cfgPath(""), __listenFD(-1), __running(false) {
    // 1. load server config if exists
    if (_cfgPath) { __cfgPath = _cfgPath; loadCfgFrom(_cfgPath); }
    else __serverCfg = defaultCfg;
    // 2. set server socket addr
    __serverAddress.sin_addr.s_addr = __serverCfg.__serverAddr;
    __serverAddress.sin_port = htons(__serverCfg.__serverPort);
    __serverAddress.sin_family = DOMAIN;
}

#include <iostream>

Server::~Server() {
    // 1. stop all thread
    if (isRunning()) stop();
    // 2. relase all resources (fd, ...)
    __ioWorkers.clear();
    __listenWorker.reset();
    if (-1 != __listenFD) close(__listenFD);
    std::cout << "server out" << std::endl;
}

/*
 * Initialize resources for server and start worker threads
 */
server_err_t Server::start() {
    int i, maxFDS, flags;
    server_err_t error;
    worker_sptr_t worker;

    error = SERVER_OK; maxFDS = __serverCfg.__maxFD;

    // 1. create server socket
    if (-1 == (__listenFD=socket(DOMAIN, SOCK_STREAM, PROTO))) return SOCKET_INIT_FAILED;
    if (-1 == (flags=fcntl(__listenFD, F_GETFL, 0))) { error = SOCKET_INIT_FAILED; goto stop;}
    fcntl(__listenFD, F_SETFL, flags | O_NONBLOCK);
    if (-1 == bind(__listenFD, (sockaddr*)&__serverAddress, sizeof(sockaddr))) { error = SOCKET_BIND_FAILED; goto stop; }
    if (-1 == listen(__listenFD, __serverCfg.__listenQueue)) { error = SOCKET_LISTEN_FAILED; goto stop; }

    // 2. create IO worker
    for (i=0; i<__serverCfg.__workerNum; ++i) {
        worker = std::make_shared<Worker>(
            std::make_unique<Epoll>(maxFDS, error), 
            std::make_unique<IOEventHandler>());
        if (SERVER_OK != error) goto stop;
        __ioWorkers.push_back(worker);
    }

    // 3. create listen worker
    __listenWorker = std::make_shared<Worker>(
        std::make_unique<Epoll>(maxFDS, error), 
        std::make_unique<ListenEventHandler>(__ioWorkers));
    if (SERVER_OK != error) goto stop;
    if (SERVER_OK != (error=__listenWorker->addEvent(
        std::make_shared<Event>(__listenFD, EPOLLIN|EPOLLET)))) goto stop;

    // 4. start all worker
    for (auto& ioWorker : __ioWorkers) {
        ioWorker->run();
        if (!ioWorker->isRunning()) { error = WORKER_START_FAILED; goto stop; }
    }
    __listenWorker->run();
    if (!__listenWorker->isRunning()) { error = WORKER_START_FAILED; goto stop; }
    __running = true;
    goto out;
stop:
    stop();
out:
    return error;
}

void Server::stop() {
    __running = false;
    for (auto& ioWorker : __ioWorkers) if (ioWorker && ioWorker->isRunning()) ioWorker->stop();
    if (__listenWorker && __listenWorker->isRunning()) __listenWorker->stop();
}

bool Server::isRunning() { return __running; }

server_err_t Server::statusChecking() {
    if (!__listenWorker->isRunning()) return LISTEN_WORKER_DUMP;
    for (auto& ioWorker : __ioWorkers) {
        if (!ioWorker->isRunning()) return IO_WORKER_DUMP;
    }
    return SERVER_OK;
}
