#include <unistd.h>
#include "Epoll.hpp"
#include "Event.hpp"
using event_sptr_t = Epoll::event_sptr_t;

bool Epoll::validEpollFD() { return __epfd >= 3; }

Epoll::Epoll(int _maxFDS, server_err_t& _errorcode): 
    __maxFDS(_maxFDS), __acceptedEvents(_maxFDS) {
    if (-1 == (__epfd=epoll_create1(0))) _errorcode = INVALID_EPOLL_FD;
    else _errorcode = SERVER_OK;
}

Epoll::~Epoll() {
    __maxFDS = 0;
    close(__epfd);
    __eventsMapLock.lock();
    __events.clear();
    __eventsMapLock.unlock();
}

int Epoll::epollAddEvent(event_sptr_t _event) {
    int fd;
    epoll_event event;
    server_err_t error;

    fd = _event->getFD();
    if (activeFDS() >= maxFDS()) return MAX_EVENT_NUM;
    if (!validEpollFD()) return INVALID_EPOLL_FD;

    error = SERVER_OK; event.data.fd = fd; event.events = _event->getEvent();
    __eventsMapLock.lock();
    if (-1 == epoll_ctl(__epfd, EPOLL_CTL_ADD, fd, &event)) { error = EPOLL_EVENT_ADD_FAILED; goto out; }
    __events[fd] = _event;
out:
    __eventsMapLock.unlock();
    return error;
}

/*
 * Remove _event from __epfd & __events
 * It means Epoll cannot response to __epfd
 */
int Epoll::epollDelEvent(event_sptr_t _event) {
    int fd;
    epoll_event event;
    server_err_t error;

    fd = _event->getFD();
    if (!validEpollFD()) return INVALID_EPOLL_FD;
    
    error = SERVER_OK; event.data.fd = fd; event.events = _event->getEvent();
    __eventsMapLock.lock();
    if (-1 == epoll_ctl(__epfd, EPOLL_CTL_DEL, fd, &event)) { error = EPOLL_EVENT_DEL_FAILED; goto out; }
    __events.erase(fd);
out:
    __eventsMapLock.unlock();
    return error;
}

int Epoll::epollModEvent(event_sptr_t _event) {
    int fd;
    epoll_event event;

    fd = _event->getFD();
    if (!validEpollFD()) return INVALID_EPOLL_FD;

    event.data.fd = fd; event.events = _event->getEvent();
    if (-1 == epoll_ctl(__epfd, EPOLL_CTL_MOD, fd, &event)) return EPOLL_EVENT_MOD_FAILED;

    return SERVER_OK;
}

std::vector<event_sptr_t> Epoll::epollWait(server_err_t& _errorcode, int _epTimeout) {
    int i, num, fd;
    event_sptr_t cur;
    std::vector<event_sptr_t> ret;

    if (!validEpollFD()) { _errorcode = INVALID_EPOLL_FD; return ret; }
    if (-1 == (num=epoll_wait(__epfd, &*__acceptedEvents.begin(), maxFDS(), _epTimeout))) {
        _errorcode = EPOLL_WAIT_FAILED; return ret;
    }

    __eventsMapLock.lock();
    for (i=0; i<num; ++i) {
        fd = __acceptedEvents[i].data.fd;
        if (!__events.count(fd)) continue;
        cur = __events[fd];
        cur->setResponseEvent(__acceptedEvents[i].events);
        ret.push_back(cur);
    }
    __eventsMapLock.unlock();

    _errorcode = SERVER_OK;
    return ret;
}

int Epoll::maxFDS() { return __maxFDS; }

int Epoll::activeFDS() {
    int ret;
    __eventsMapLock.lock(); ret = __events.size(); __eventsMapLock.unlock();
    return ret;
}
