#pragma once

#include "EventHandler.hpp"

struct IRouter;
struct IResourceAccessor;

/*
 * IOEventHandler
 * IOEventHandler do the following three things:
 * 1. Get HttpRequest when clientFD is readable. It use HttpRequestAnalyser to
 *    read request data from clientFD.
 * 2. Analyse HttpRequest and generate corresponding HttpReponses(when file is
 *    too large), it use a HttpResponsor to generate HttpResponses. The HttpRe
 *    sponsor firstly use a Router to map uri into a concrete Resource object,
 *    then responsor build the body part of HttpResponses by using ResourceAcc
 *    essor.
 * 3. Send generated HttpResponses back to client.
 * The IOEventHandler needs three things to accomplish its work: clientFD, Rou
 * ter and ResourceAccessor. Cause Router and ResourceAccessor should be owned
 * by Server and shared among all IOEventHandlers, the Server needs to pass it
 * into IOEventHandler during initialize stage.
 */
class IOEventHandler: public IEventHandler {
    using router_sptr_t     = std::shared_ptr<IRouter>;
    using accessor_sptr_t   = std::shared_ptr<IResourceAccessor>;
public:
    IOEventHandler(router_sptr_t, accessor_sptr_t);
    IOEventHandler(const IOEventHandler&) = delete;
    ~IOEventHandler();
    virtual int handle(event_sptr_t);
private:
    router_sptr_t       __router;
    accessor_sptr_t     __accessor;
};
