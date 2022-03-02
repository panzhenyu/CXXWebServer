#pragma once

#include <memory>
#include <cstdint>
#include <netinet/in.h>

class Worker;

class Event {
    int         __fd;
    uint32_t    __ev;
    uint32_t    __rev;
    uint32_t    __lastev;
    Worker*     __worker;   // weak pointer
public:
    Event(int, uint32_t);
    Event(const Event&) = delete;
    ~Event();
    int getFD();
    uint32_t getEvent();
    uint32_t getResponseEvent();
    uint32_t getLastEvent();
    Worker* getWorker();
    void setEvent(uint32_t);
    void setResponseEvent(uint32_t);
    void setLastEvent(uint32_t);
    void setWorker(Worker*);
};
