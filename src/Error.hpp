#pragma once

using server_err_t = unsigned;

enum Error {
    SERVER_OK, 
    SOCKET_INIT_FAILED, 
    SOCKET_BIND_FAILED, 
    SOCKET_LISTEN_FAILED, 
    SAME_CLIENT_FD, 
    INVALID_EPOLL_FD, 
    MAX_EVENT_NUM, 
    WORKER_START_FAILED, 
    WORKER_ALREADY_RUNNING, 
    EPOLL_EVENT_ADD_FAILED, 
    EPOLL_EVENT_DEL_FAILED, 
    EPOLL_EVENT_MOD_FAILED, 
    EPOLL_WAIT_FAILED, 
};

static const char* errMsg[] = {
    "OK",
    "Socket Init Failed",
};
