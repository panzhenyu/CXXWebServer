#include "Server.hpp"
#include "Error.hpp"
#include "Epoll.hpp"
#include "Event.hpp"
#include "Worker.hpp"
#include "EventHandler.hpp"
#include <iostream>
#include "HttpUtils/HttpRequest.hpp"
using namespace std;

int main() {
    Server s;
    server_err_t error;
    if (SERVER_OK != (error=s.start())) { cout << error << endl; return -1; }
    cout << "server started" << endl;
    while (true);
    return 0;
}
