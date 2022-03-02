#pragma once

#include <memory>

class Event;

struct IEventHandler {
    using event_sptr_t = std::shared_ptr<Event>;
    virtual int handle(event_sptr_t) = 0;
};
