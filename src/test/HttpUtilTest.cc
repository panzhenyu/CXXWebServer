#include <iostream>
#include "../HttpUtils/Router.hpp"
#include "../HttpUtils/HttpRequest.hpp"
#include "../HttpUtils/SocketStream.hpp"
#include "../HttpUtils/HttpResponse.hpp"
#include "../HttpUtils/ResourceAccessor.hpp"
#include "../Error.hpp"
#include "../Server.hpp"
#include "../Epoll.hpp"

using namespace std;

void test4Request(int __clientFD) {
    server_err_t error;
    shared_ptr<SocketInputStream> iss;
    shared_ptr<SocketOutputStream> oss;
    HttpRequestAnalyser analyser;
    HttpResponsor responsor;
    shared_ptr<HttpRequest> req;
    shared_ptr<HttpResponse> res;
    string msg;
    Router router;
    CachedResourceAccessor accessor(nullptr);

    iss = make_shared<SocketInputStream>(make_shared<SocketStreamBuffer>(__clientFD));
    oss = make_shared<SocketOutputStream>(make_shared<SocketStreamBuffer>(__clientFD));
    // while (*iss) {
    //     getline(*iss, msg, '\n');
    //     msg.pop_back();
    //     cout << msg << "|" << endl;
    // }
    while (!iss->eof()) {
        req = analyser.getOneHttpRequest(*iss, error);
        cout << "getOneHttpRequest returned with error: " << error << endl;
        cout << *req << endl;
        res = responsor.getResponseFromRequest(*req, router, accessor);
        error = responsor.response(*oss, *res);
        cout << "response ret with error: " << error << endl;
    }
}

int main() {
    socklen_t socketLen;
    int __listenFD, clientFD;
    sockaddr_in __serverAddress, cad;

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
    cout << "client in: " << clientFD << endl;
    test4Request(clientFD);
    while (true);
}
