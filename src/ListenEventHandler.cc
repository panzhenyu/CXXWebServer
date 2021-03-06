#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include "Event.hpp"
#include "Worker.hpp"
#include "Log/Logger.hpp"
#include "ListenEventHandler.hpp"

ListenEventHandler::ListenEventHandler(std::vector<std::shared_ptr<Worker>>& _workers) {
    for (auto& spWorker : _workers) __ioWorkers.push_back(spWorker);
}

int ListenEventHandler::handle(event_sptr_t _event) {
    // distribute client fd into io workers
    size_t i;
    std::shared_ptr<Worker> cur;
    int listenedFD, clientFD, flags;

    listenedFD = _event->getFD(); clientFD = -1;
    while (-1 != (clientFD=accept(listenedFD, NULL, NULL))) {
        // 1. set non-block fd & build Event
        if (-1 == (flags=fcntl(clientFD, F_GETFL, 0))) { close(clientFD); continue; }
        fcntl(clientFD, F_SETFL, flags | O_NONBLOCK);

        // 2. find an IO Worker
        for (i=0; i<__ioWorkers.size(); ++i) {
            cur = __ioWorkers[i].lock();
            if (cur->isRunning() && cur->activeFDS()<cur->maxFDS()) break;
        }

        // 3. add Event to Worker, move Worker into list tail if add succeedly
        if (i==__ioWorkers.size() || SERVER_OK!=cur->addEvent(std::make_shared<Event>(clientFD, EPOLLET|EPOLLIN))) {
            close(clientFD); continue;
        }
        __ioWorkers.erase(__ioWorkers.begin()+i);
        __ioWorkers.push_back(cur);

        LOG2DIARY << "Accept client with clientFD and assign it to worker: " << cur->getThreadID() << '\n';
    }
    return SERVER_OK;
}
