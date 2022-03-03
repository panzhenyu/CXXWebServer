#include <iostream>
#include <unistd.h>
#include "Epoll.hpp"
#include "Event.hpp"
#include "Worker.hpp"
#include "EventHandler.hpp"

#include <cassert>

using event_sptr_t = Worker::event_sptr_t;

Worker::Worker(epoll_uptr_t&& _epoller, handler_uptr_t&& _handler):
    __running(false), 
    __eventPoller(std::move(_epoller)), 
    __eventHandler(std::move(_handler)) {
    assert(__eventPoller);
    assert(__eventHandler);
}

Worker::~Worker() {
    stop();
    __eventPoller.reset();
    __eventHandler.reset();
}

int Worker::addEvent(event_sptr_t _pEvent) {
    server_err_t error;

    _pEvent->setWorker(this);
    error = __eventPoller->epollAddEvent(_pEvent);
    if (SERVER_OK != error) _pEvent->setWorker(nullptr);

    return error;
}

/*
 * 意味着Worker不再接收该事件的响应
 * 若该事件正在被handleEvent处理，则处理后由智能指针自动关闭事件
 */
int Worker::delEvent(event_sptr_t _pEvent) { return __eventPoller->epollDelEvent(_pEvent); }

int Worker::modEvent(event_sptr_t _pEvent) { return __eventPoller->epollModEvent(_pEvent); }

std::vector<event_sptr_t> Worker::eventPoll(server_err_t& _errorcode, int _epTimeout) {
    return __eventPoller->epollWait(_errorcode, _epTimeout);
}

int Worker::maxFDS() { return __eventPoller->maxFDS(); }

int Worker::activeFDS() { return __eventPoller->activeFDS(); }

int Worker::run() {
    if (__running) return WORKER_ALREADY_RUNNING;
    __running = true;
    std::thread t([this] () {
        server_err_t error;
        while (isRunning()) {
            auto events = eventPoll(error, -1);
            if (SERVER_OK != error) break;
            for (auto& e : events) {
                handleEvent(e);
            }
            events.clear();
        }
        __running = false;
        std::cout << "worker exit with err " << error << std::endl;
        return;
    });
    t.detach(); __threadID = t.get_id();
    if (!isRunning()) return WORKER_START_FAILED;
    return SERVER_OK;
}

/*
 * Destroy the running thread
 * But all resources will be reserved
 */
void Worker::stop() { __running = false; }

void Worker::handleEvent(event_sptr_t _event) { __eventHandler->handle(_event); }

bool Worker::isRunning() { return __running; }

Worker::tid_t Worker::getThreadID() { return __threadID; }
