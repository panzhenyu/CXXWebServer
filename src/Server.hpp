#pragma once

#include <string>
#include <vector>
#include <atomic>
#include <memory>
#include <cstdint>
#include <unistd.h>
#include <netinet/in.h>
#include <unordered_map>

#define DOMAIN      AF_INET
#define PROTO       IPPROTO_TCP

struct ServerConfig {
    uint32_t __maxFD;
    uint16_t __clientTimeoutSecond;
    uint16_t __workerNum;
    uint32_t __serverAddr;
    uint16_t __serverPort;
    uint16_t __listenQueue;
};

static ServerConfig defaultCfg = {
    .__maxFD = 100000, 
    .__clientTimeoutSecond = 60, 
    .__workerNum = 7, 
    .__serverAddr = INADDR_ANY, 
    .__serverPort = 8888, 
    .__listenQueue = 1000, 
};

class Event;
class Worker;

class Server {
public:
    using event_sptr_t  = std::shared_ptr<Event>;
    using worker_sptr_t = std::shared_ptr<Worker>;
private:
    /* server config */
    std::string                 __cfgPath;
    ServerConfig                __serverCfg;
    /* running config */
    std::atomic<bool>           __running;
    // about client
    std::vector<worker_sptr_t>  __ioWorkers;
    // server socket
    int __listenFD;
    worker_sptr_t               __listenWorker;
    sockaddr_in                 __serverAddress;
private:
    void loadCfgFrom(std::string);
public:
    Server();
    Server(const char* _cfgPath);
    Server(const Server&) = delete;
    ~Server();
    int start();
    int stop();
    bool isRunning();
};
