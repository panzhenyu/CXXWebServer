#include <unistd.h>
#include <iostream>
#include "Event.hpp"
#include "Worker.hpp"
#include "IOEventHandler.hpp"
#include "HttpUtils/SocketStream.hpp"

#include <cassert>

IOEventHandler::IOEventHandler() {}

IOEventHandler::IOEventHandler(const IOEventHandler&) {}

IOEventHandler::~IOEventHandler() {}

int IOEventHandler::handle(event_sptr_t _event) {
    int fd;
    std::string line;
    std::unique_ptr<SocketInputStream> in;
    std::unique_ptr<SocketOutputStream> out;
    fd = _event->getFD();
    
    in = std::make_unique<SocketInputStream>(std::make_shared<SocketStreamBuffer>(fd));
    out = std::make_unique<SocketOutputStream>(std::make_shared<SocketStreamBuffer>(fd));
    getline(*in, line);
    
    if (line[0] == 'G')
        // *out << "HTTP/1.0" << " " << 200 << " " << "OK" << "\n"
        //     << "Content-Length: 5" << "\n" << "\n" << "Hello";
        send(fd, "HTTP/1.0 200 OK\nContent-Length: 5\n\nHello", 41, 0);

    assert(_event);
    assert(_event->getWorker());
    _event->getWorker()->delEvent(_event);
    
    return SERVER_OK;
}
