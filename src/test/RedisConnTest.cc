#include <string>
#include <memory>
#include <iostream>
#include "../Cache/RedisCache.hpp"

using namespace std;
using pool_sptr_t = RedisCache::poll_sptr_t;
using conn_sptr_t = RedisConnectionPool::conn_sptr_t;

int main() {
    server_err_t error;
    string key, val, cmd;
    RedisConnectionBuilder builder;
    conn_sptr_t conn = builder.setIP("127.0.0.1").setPort(6379).build();
    if (SERVER_OK != (error=conn->contextInit())) { cout << error << endl; return -1; }
    do {
        cin >> cmd;
        if (cmd == "OUT") break;
        else if (cmd == "GET") {
            cin >> key;
            if (conn->getKey(key, val)) cout << val << endl;
            else cout << "FAILED" << endl; 
        } else if (cmd == "PUT") {
            cin >> key >> val;
            if (conn->putKey(key, val)) cout << "OK" << endl;
            else cout << "FAILED" << endl;
        }
    } while (true);
    return 0;
}
