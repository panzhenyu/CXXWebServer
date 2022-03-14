#include <iostream>
#include <sys/signal.h>
#include "Server.hpp"
#include "Error.hpp"
#include "Epoll.hpp"
#include "Event.hpp"
#include "Worker.hpp"
#include "EventHandler.hpp"
#include "HttpUtils/HttpRequest.hpp"
#include "Log/AsyncLogger.hpp"
using namespace std;

void handleSIGPIPE(int) {}

int main() {
    Server s;
    server_err_t error;

    // AsyncLogger::getAsyncLogger().setLogFile("./log.txt");
    signal(SIGPIPE, handleSIGPIPE);
    if (SERVER_OK != (error=s.start())) { cout << error << endl; return -1; }
    cout << "server started" << endl;
    while (SERVER_OK == (error=s.statusChecking())) {
        this_thread::sleep_for(chrono::seconds(10));
        cout << "server status checking with result " << error << endl;
    }
    return 0;
}
