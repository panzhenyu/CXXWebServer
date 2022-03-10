#include <thread>
#include <iostream>
#include <algorithm>
#include "AsyncLogger.hpp"

AsyncLogger& AsyncLogger::getAsyncLogger() {
    static AsyncLogger logger;
    return logger;
}

AsyncLogger::AsyncLogger(): __overflow(DEFAULT_OVERFLOW), __running(false) {
    setLogFile(DEFAULT_LOG);
    // auto threadFunc = std::bind(&AsyncLogger::run, this);

    // __running = true;
    // std::thread t([&]() {
    //     threadFunc();
    //     stop();
    // });
    // __threadID = t.get_id();
    // t.detach();
}

void AsyncLogger::run() {
    server_err_t error = SERVER_OK;
    while (isRunning() && SERVER_OK==error && __out.is_open()) {
        std::unique_lock<std::mutex> alternateLck(__lockAlternate);
        while (__alternate.size() == 0) {
            __waitAlternateWritable.wait_for(alternateLck, std::chrono::seconds(10));
            if (__alternate.size()==0 && __lockCurrent.try_lock()) {
                if (__current.size()) __alternate.swap(__current);
                __lockCurrent.unlock();
            }
        }
        error = flushAlternate2LogFile();
        __alternate.clear();
        __waitAlternateWriteDone.notify_one();
    }
}

bool AsyncLogger::currentAvailable4(ch_buff_t& _buff) {
    return __current.empty() || __current.back().rest()<=_buff.length() || __current.size()<__overflow;
}

void AsyncLogger::append2Current(ch_buff_t&& _buff) {
    if (__current.empty()) __current.push_back(std::move(_buff));
    else if (__current.back().rest() >= _buff.length())
        __current.back().append(_buff.begin(), _buff.cur());
    else __current.push_back(std::move(_buff));
}

server_err_t AsyncLogger::flushAlternate2LogFile() {
    server_err_t error = SERVER_OK;
    if (!__out.is_open()) return LOGFILE_NOT_OPEN;
    for (auto& buffer : __alternate) {
        if (__out.write(buffer.begin(), buffer.length()).fail()) {
            error = LOGFILE_WRITE_FAILE;
            stop(); break;
        }
    }
    __out.flush();
    __out.close();
    return error;
}

AsyncLogger::~AsyncLogger() {
    if (isRunning()) stop();
    while (true) {
        if (__alternate.size()) {
            std::unique_lock<std::mutex> alternateLck(__lockAlternate);
            flushAlternate2LogFile();
            __alternate.clear();
            __waitAlternateWriteDone.notify_one();
        } else if (__lockCurrent.try_lock()) {
            if (__current.size()) __alternate.swap(__current);
            __lockCurrent.unlock();
        }
        if (!__alternate.size() && !__current.size()) break;
    }
    __out.flush();
    __out.close();
}

void AsyncLogger::stop() { __running = false; }

bool AsyncLogger::isRunning() { return __running; }

bool AsyncLogger::setOverflowSize(unsigned _of) {
    if (_of == 0) return false;
    __overflow = _of;
    return true;
}

const std::string& AsyncLogger::getLogFile() { return __logFile; }

server_err_t AsyncLogger::setLogFile(const std::string& _path) {
    server_err_t error = SERVER_OK;
    std::lock_guard<std::mutex> currentLck(__lockAlternate);
    if (__out.is_open()) {
        error = flushAlternate2LogFile();
        __out.close();
    }
    __out.open(_path, std::ofstream::app|std::ofstream::out);
    if (!__out.is_open()) error = LOGFILE_OPEN_FAILED;
    return error;
}

server_err_t AsyncLogger::append(ch_buff_t&& _b) {
    std::cout << _b.length() << std::endl;
    std::cout << _b.begin() << std::endl;
    append2Current(std::move(_b));
    if (!isRunning()) return LOG_THREAD_STOPPED;

    std::lock_guard<std::mutex> currentLck(__lockCurrent);
    if (currentAvailable4(_b)) append2Current(std::move(_b));
    else {
        std::unique_lock<std::mutex> alternateLck(__lockAlternate);
        while (__alternate.size()) __waitAlternateWriteDone.wait(alternateLck);
        __current.swap(__alternate);
        __waitAlternateWritable.notify_one();

        if (!currentAvailable4(_b)) return BUFFER_NOTAVAIL_AFTER_SWAP;
        else __current.push_back(std::move(_b));
    }
    return SERVER_OK;
}
