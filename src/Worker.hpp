#pragma once

#include <atomic>
#include <memory>
#include <thread>
#include <vector>
#include "Error.hpp"

class Epoll;
class Event;
class IEventHandler;

class Worker {
public:
    using event_sptr_t = std::shared_ptr<Event>;
    using epoll_uptr_t = std::unique_ptr<Epoll>;
    using handler_uptr_t = std::unique_ptr<IEventHandler>;
    using tid_t = std::thread::id;
private:
    tid_t                   __threadID;
    std::atomic<bool>       __running;
    epoll_uptr_t            __eventPoller;
    handler_uptr_t          __eventHandler;
    /* 
     * 反向代理：
     * 再添加一个poller监听子服务器，并维护客户端fd、服务器端fd的map
     * 当子服务器响应，根据服务器端fd获取客户端fd，将结果回送
     * 还需维护一个全局的服务器端fd链接池，每次使用完毕后归还fd?
     * 也可以考虑将上述内容维护在EventHandler对象中?
     */
public:
    Worker(epoll_uptr_t&&, handler_uptr_t&&);
    Worker(const Worker&) = delete;
    ~Worker();
    int addEvent(event_sptr_t);
    int delEvent(event_sptr_t);
    int modEvent(event_sptr_t);
    std::vector<event_sptr_t> eventPoll(server_err_t&, int);
    int maxFDS();
    int activeFDS();

    int run();
    void stop();
    void handleEvent(event_sptr_t);
    
    bool isRunning();
    tid_t getThreadID();
};
