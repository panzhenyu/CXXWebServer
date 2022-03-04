#include <iostream>
#include "../HttpUtils/SocketStream.hpp"
#include "../Error.hpp"
#include "../Server.hpp"
#include "../Epoll.hpp"

using namespace std;

int main() {
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
    if (-1 == listen(__listenFD, 5)) { close(__listenFD); return SOCKET_LISTEN_FAILED; }
    if (-1 == (clientFD=accept(__listenFD, (sockaddr*)&cad, &socketLen))) {
        printf("Accept failed with errno: %d\n", errno);
        close(__listenFD);
        return -1;
    }
    
    ssize_t recvlen, i;
    char buff[4096];
    recvlen =  recv(clientFD, buff, 4096, 0);
    for (i=0; i<recvlen; ++i) printf("%c", buff[i]);
    printf("\n");

    // iss = std::make_unique<SocketInputStream>(std::make_shared<SocketStreamBuffer>(clientFD));
    // oss = std::make_unique<SocketOutputStream>(std::make_shared<SocketStreamBuffer>(clientFD));
    // while (iss && !iss->eof()) {
    //     getline(*iss, msg);
    //     cout << msg << endl;
    //     *oss << msg << '\n';
    // }
    // close(clientFD);
    // close(__listenFD);
    while(1);
}
