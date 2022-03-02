#pragma once

#include <vector>
#include "EventHandler.hpp"

class Worker;

class ListenEventHandler: public IEventHandler {
    // accept client fd and distribute it to IO workers
    std::vector<std::weak_ptr<Worker>> __ioWorkers;
    // need a worker distribute algorithm
public:
    ListenEventHandler(std::vector<std::shared_ptr<Worker>>&);
    ListenEventHandler(const ListenEventHandler&) = delete;
    ~ListenEventHandler();
    virtual int handle(event_sptr_t);
};
