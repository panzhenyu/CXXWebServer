#pragma once

#include "EventHandler.hpp"

class IOEventHandler: public IEventHandler {
    // use analylser
public:
    IOEventHandler();
    IOEventHandler(const IOEventHandler&);
    ~IOEventHandler();
    virtual int handle(event_sptr_t);
};
