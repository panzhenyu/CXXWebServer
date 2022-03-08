#ifndef _EPOLL_HPP
#define _EPOLL_HPP

#include <mutex>
#include <atomic>
#include <memory>
#include <vector>
#include <sys/epoll.h>
#include <unordered_map>
#include "Error.hpp"

class Event;

class Epoll {
public:
    using event_sptr_t = std::shared_ptr<Event>;
private:
    int __epfd;
    std::atomic<int> __maxFDS;
    std::vector<epoll_event> __acceptedEvents;
    std::unordered_map<int, event_sptr_t> __events;  // fd -> shared_ptr<Event>
    std::mutex __eventsMapLock;
public:
    Epoll(int, server_err_t&);
    Epoll(const Epoll&) = delete;
    ~Epoll();
    int epollAddEvent(event_sptr_t);
    int epollDelEvent(event_sptr_t);
    int epollModEvent(event_sptr_t);
    std::vector<event_sptr_t> epollWait(server_err_t&, int);
    int maxFDS();
    int activeFDS();
    bool validEpollFD();
};

#endif
