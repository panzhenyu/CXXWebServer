#include <cassert>
#include <unistd.h>
#include <iostream>
#include "Event.hpp"
#include "Worker.hpp"
#include "IOEventHandler.hpp"
#include "HttpUtils/HttpRequest.hpp"
#include "HttpUtils/SocketStream.hpp"
#include "HttpUtils/HttpResponse.hpp"

IOEventHandler::IOEventHandler(router_sptr_t _router, accessor_sptr_t _accessor):
__router(_router), __accessor(_accessor) {}

IOEventHandler::~IOEventHandler() {}

int IOEventHandler::handle(event_sptr_t _event) {
    int fd;
    std::string line;
    server_err_t error;
    HttpResponsor responsor;
    HttpRequestAnalyser analyser;
    std::shared_ptr<HttpRequest>req;
    std::shared_ptr<HttpResponse> res;
    std::shared_ptr<SocketInputStream> in;
    std::shared_ptr<SocketOutputStream> out;

    fd = _event->getFD();
    in = std::make_shared<SocketInputStream>(std::make_shared<SocketStreamBuffer>(fd));
    out = std::make_shared<SocketOutputStream>(std::make_shared<SocketStreamBuffer>(fd));
    while (!in->eof()) {
        req = analyser.getOneHttpRequest(*in, error);
        std::cout << "getOneHttpRequest returned with error: " << error << std::endl;
        if (SERVER_OK == error) {
            std::cout << *req << std::endl;
            res = responsor.getResponseFromRequest(*req, *__router, *__accessor);
            error = responsor.response(*out, *res);
            std::cout << "response ret with error: " << error << std::endl;
        }
    }

    assert(_event);
    assert(_event->getWorker());
    _event->getWorker()->delEvent(_event);
    
    return SERVER_OK;
}
