#include <thread>
#include <algorithm>
#include "AsyncLogger.hpp"

AsyncLogger& AsyncLogger::getAsyncLogger() {
    static AsyncLogger logger;
    return logger;
}

AsyncLogger::AsyncLogger(): 
__overflow(DEFAULT_OVERFLOW), __secondTimeout(DEFAULT_TIMEOUT), __running(false) {
    auto threadFunc = std::bind(&AsyncLogger::run, this);

    __running = true;
    std::thread t([threadFunc, this]() {
        threadFunc();
        stop();
    });
    __threadID = t.get_id();
    t.detach();
}

void AsyncLogger::run() {
    std::cv_status waitStatus = std::cv_status::no_timeout;
    while (isRunning()) {
        std::unique_lock<buff_mtx_t> bufferLck(__lockBuff);
        waitStatus = __wait4Flush.wait_for(bufferLck, std::chrono::seconds(__secondTimeout));

        if (__alternate.size()) __loggerLocal.swap(__alternate);
        if (std::cv_status::timeout == waitStatus){
            __loggerLocal.insert(__loggerLocal.end(), std::make_move_iterator(__current.begin()),
                std::make_move_iterator(__current.end()));
            __current.clear();
            waitStatus = std::cv_status::no_timeout;
        }
        bufferLck.unlock();

        std::lock_guard<file_mtx_t> fileLck(__lockLogFile);
        flush2LogFile(__loggerLocal);
        __loggerLocal.clear();
    }
}

bool AsyncLogger::currentAvailable4(ch_buff_t& _buff) {
    return  __current.size() < __overflow ||
            __current.back().rest() >= _buff.length();
}

void AsyncLogger::append2Current(ch_buff_t&& _buff) {
    if (__current.empty() || __current.back().rest()<_buff.length()) {
        __current.push_back(std::move(_buff));
    } else  __current.back().append(_buff.begin(), _buff.cur());
}

/*
 * AsyncLogger::flush2LogFile
 * Flush all contents of _buffs into current log file.
 * If log file isn't opened, return an error.
 * If write failed, try to reopen.
 */
server_err_t AsyncLogger::flush2LogFile(buffs_t& _buffs) {
    server_err_t error;
    if (!__out.is_open()) return LOGFILE_OPEN_FAILED;

    error = SERVER_OK;
    for (auto& buffer : _buffs) {
        if (__out.write(buffer.begin(), buffer.length()).fail()) {
            error = LOGFILE_WRITE_FAILE;
            setLogFile(getLogFile());
        }
    }
    __out.flush();

    return error;
}

AsyncLogger::~AsyncLogger() {
    if (isRunning()) stop();
    __wait4Flush.notify_all();

    std::lock_guard<buff_mtx_t> bufferLck(__lockBuff);
    std::lock_guard<file_mtx_t> fileLck(__lockLogFile);
    flush2LogFile(__loggerLocal);
    flush2LogFile(__alternate);
    flush2LogFile(__current);
    __current.clear();
    __alternate.clear();
    __loggerLocal.clear();
    __out.flush();
    __out.close();
}

void AsyncLogger::stop() { __running = false; }

bool AsyncLogger::isRunning() { return __running; }

bool AsyncLogger::logFileOn() {
    std::lock_guard<file_mtx_t> fileLck(__lockLogFile);
    return __out.is_open();
}

bool AsyncLogger::setTimeoutSecond(unsigned _s) {
    if (0 == _s) return false;
    __secondTimeout = _s;
    return true;
}

bool AsyncLogger::setOverflowSize(unsigned _of) {
    if (_of == 0) return false;
    __overflow = _of;
    return true;
}

const std::string& AsyncLogger::getLogFile() { return __logFile; }

server_err_t AsyncLogger::setLogFile(const std::string& _path) {
    std::lock_guard<file_mtx_t> fileLck(__lockLogFile);
    if (__out.is_open()) {
        __out.flush();
        __out.close();
    }
    __out.open(_path, std::ofstream::app|std::ofstream::out);
    return __out.is_open() ? SERVER_OK: LOGFILE_OPEN_FAILED;
}

server_err_t AsyncLogger::append(ch_buff_t&& _b) {
    if (!isRunning()) return LOG_THREAD_STOPPED;
    std::lock_guard<buff_mtx_t> bufferLck(__lockBuff);
    if (!currentAvailable4(_b)) {
        for (auto& buffer : __current) __alternate.push_back(std::move(buffer));
        __current.clear();
    }
    if (__alternate.size()) __wait4Flush.notify_one();
    append2Current(std::move(_b));

    return SERVER_OK;
}
