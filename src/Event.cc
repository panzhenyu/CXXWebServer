#include <unistd.h>
#include "Event.hpp"

Event::Event(int _fd, uint32_t _event):
    __fd(_fd), __ev(_event), __rev(0), __lastev(0), __worker(nullptr) {}

Event::~Event() { close(__fd); __worker = nullptr; }

int Event::getFD() { return __fd; }

uint32_t Event::getEvent() { return __ev; }

uint32_t Event::getResponseEvent() { return __rev; }

uint32_t Event::getLastEvent() { return __lastev; }

Worker* Event::getWorker() { return __worker; }

void Event::setEvent(uint32_t _ev) { __ev = _ev; }

void Event::setResponseEvent(uint32_t _rev) { __rev = _rev; }

void Event::setLastEvent(uint32_t _lastev) { __lastev = _lastev; }

void Event::setWorker(Worker* _worker) { __worker = _worker; }
