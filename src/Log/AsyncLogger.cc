#include <thread>
#include <algorithm>
#include "AsyncLogger.hpp"

AsyncLogger& AsyncLogger::getAsyncLogger() {
    static AsyncLogger logger;
    return logger;
}

AsyncLogger::AsyncLogger(): __overflow(DEFAULT_OVERFLOW), __running(false) {
    auto threadFunc = std::bind(&AsyncLogger::run, this);

    __running = true;
    std::thread t([&]() {
        threadFunc();
        stop();
    });
    __threadID = t.get_id();
    t.detach();
}

void AsyncLogger::run() {
    while (isRunning()) {
        std::unique_lock<std::mutex> alternateLck(__lockAlternate);
        while (__alternate.size() == 0) {
            __waitAlternateWritable.wait_for(alternateLck, std::chrono::seconds(10));
            if (__alternate.size()==0 && __lockCurrent.try_lock()) {
                if (__current.size()) __alternate.swap(__current);
                __lockCurrent.unlock();
            }
        }
        flushAlternate2LogFile();
        __alternate.clear();
        __waitAlternateWriteDone.notify_one();
    }
}

bool AsyncLogger::currentAvailable4(ch_buff_t& _buff) {
    return __current.empty() || __current.back().rest()<=_buff.length() || __current.size()<__overflow;
}

void AsyncLogger::append2Current(ch_buff_t&& _buff) {
    auto rest = __current.back().rest();
    if (rest >= _buff.length()) {
        __current.back().append(_buff.begin(), _buff.cur());
    } else __current.push_back(std::move(_buff));
}

server_err_t AsyncLogger::flushAlternate2LogFile() {
    if (!__out.is_open()) return LOGFILE_NOT_OPEN;
    for (auto& buffer : __alternate) {
        
    }
    return SERVER_OK;
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
    server_err_t error;

    std::lock_guard<std::mutex> currentLck(__lockAlternate);
    if (__out.is_open()) { flushAlternate2LogFile(); __out.close(); }
    __out.open(_path, std::ofstream::app|std::ofstream::out);
    if (!__out.is_open()) return LOGFILE_OPEN_FAILED;
    return SERVER_OK;
}

server_err_t AsyncLogger::append(ch_buff_t&& b) {
    std::lock_guard<std::mutex> currentLck(__lockCurrent);
    if (currentAvailable4(b)) append2Current(std::move(b));
    else {
        if (!isRunning()) return LOG_THREAD_STOPPED;

        std::unique_lock<std::mutex> alternateLck(__lockAlternate);
        while (__alternate.size()) __waitAlternateWriteDone.wait(alternateLck);
        __current.swap(__alternate);
        __waitAlternateWritable.notify_one();

        if (!currentAvailable4(b)) return BUFFER_NOTAVAIL_AFTER_SWAP;
        else __current.push_back(std::move(b));
    }
    return SERVER_OK;
}
