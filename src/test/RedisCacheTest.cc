#include <string>
#include <memory>
#include <iostream>
#include "../Cache/RedisCache.hpp"

using namespace std;
using pool_sptr_t = RedisCache::poll_sptr_t;
using conn_sptr_t = RedisConnectionPool::conn_sptr_t;

int main() {
    conn_sptr_t conn;
    pool_sptr_t pool;
    server_err_t error;
    string key, val, cmd;

    pool = make_shared<RedisConnectionPool>(3, 1);

    conn = RedisConnectionBuilder().setIP("127.0.0.1").setPort(6379).setPasswd("panda").build();
    if (SERVER_OK != (error=conn->contextInit())) { cout << error << endl; return -1; }
    if (SERVER_OK != (error=pool->pushConnection(conn))) { cout << error << endl; return -1; }

    conn = RedisConnectionBuilder().setIP("127.0.0.1").setPort(6379).setPasswd("panda").build();
    if (SERVER_OK != (error=conn->contextInit())) { cout << error << endl; return -1; }
    if (SERVER_OK != (error=pool->pushConnection(conn))) { cout << error << endl; return -1; }

    conn = RedisConnectionBuilder().setIP("127.0.0.1").setPort(6379).setPasswd("panda").build();
    if (SERVER_OK != (error=conn->contextInit())) { cout << error << endl; return -1; }
    if (SERVER_OK != (error=pool->pushConnection(conn))) { cout << error << endl; return -1; }

    cout << "pool conn num: " << (int)(pool->curConnNum()) << endl;
    
    RedisCache cache(pool);

    do {
        cin >> cmd;
        if (cmd == "OUT") break;
        else if (cmd == "GET") {
            cin >> key;
            if (cache.get(key, val)) cout << val << endl;
            else cout << "FAILED" << endl; 
        } else if (cmd == "SET") {
            cin >> key >> val;
            if (cache.put(key, val)) cout << "OK" << endl;
            else cout << "FAILED" << endl;
        }
    } while (true);

    return 0;
}
