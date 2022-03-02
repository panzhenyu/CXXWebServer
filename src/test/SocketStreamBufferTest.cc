#include <iostream>
#include "../HttpUtils/SocketStream.hpp"
#include "../Error.hpp"
#include "../Server.hpp"
#include "../Epoll.hpp"

using namespace std;

int main() {
    server_err_t error;
    socklen_t socketLen;
    int __listenFD, clientFD;
    sockaddr_in __serverAddress, cad;
    unique_ptr<SocketInputStream> iss;
    unique_ptr<SocketOutputStream> oss;
    string msg;

    __serverAddress.sin_addr.s_addr = INADDR_ANY;
    __serverAddress.sin_port = htons(8888);
    __serverAddress.sin_family = DOMAIN;
    if (-1 == (__listenFD=socket(DOMAIN, SOCK_STREAM, PROTO))) return SOCKET_INIT_FAILED;
    if (-1 == bind(__listenFD, (sockaddr*)&__serverAddress, sizeof(sockaddr))) return SOCKET_BIND_FAILED;
    if (-1 == listen(__listenFD, 5)) return SOCKET_LISTEN_FAILED;
    if (-1 == (clientFD=accept(__listenFD, (sockaddr*)&cad, &socketLen))) return -1;

    iss = std::make_unique<SocketInputStream>(std::make_shared<SocketStreamBuffer>(clientFD));
    oss = std::make_unique<SocketOutputStream>(std::make_shared<SocketStreamBuffer>(clientFD));
    while (iss && !iss->eof()) {
        getline(*iss, msg);
        cout << msg << endl;
        *oss << msg << '\n';
    }
    *oss << msg << '\n';        // why not trigger sigpipe ?
    *oss << msg << '\n';
    // cout << "final send rets " << send(clientFD, "abc", 3, 0) << endl;
}
