#pragma once

#include <mutex>
#include <thread>
#include <atomic>
#include <memory>
#include <fstream>
#include <condition_variable>
#include "Buffer.hpp"
#include "../Error.hpp"

#define DEFAULT_LOG "/dev/null"

/*
 * AsyncLogger
 * Performed in a singleton pattern.
 * AsyncLogger will be accessed by many threads, so it must enable thread-safe interfaces.
 */
class AsyncLogger {
public:
    using atm_uint_t    = std::atomic<unsigned>;
    using atm_bool_t    = std::atomic<bool>;
    using file_mtx_t    = std::recursive_mutex;
    using buff_mtx_t    = std::mutex;
    using ch_buff_t     = Buffer<char>;
    using buffs_t       = std::vector<ch_buff_t>;
    using cond_t        = std::condition_variable;
    using tid_t         = std::thread::id;
public:
    static constexpr unsigned DEFAULT_OVERFLOW  = 2;
    static constexpr unsigned DEFAULT_TIMEOUT   = 10;
    static AsyncLogger& getAsyncLogger();
protected:
    AsyncLogger();
    AsyncLogger(const AsyncLogger&) = delete;

    void run();
    bool currentAvailable4(ch_buff_t& _buff);
    void append2Current(ch_buff_t&&);
    server_err_t flush2LogFile(buffs_t&);
public:
    ~AsyncLogger();

    void stop();
    bool isRunning();
    bool logFileOn();
    bool setTimeoutSecond(unsigned);
    bool setOverflowSize(unsigned);
    const std::string& getLogFile();

    server_err_t setLogFile(const std::string&);
    server_err_t append(ch_buff_t&&);
private:
    /*
     * overflow describe the max size of __current
     * if the __current.size() >= overflow, Logger
     * will swap __current and __alternate, and in
     * voke running thread to flush __alternate to 
     * the log file.
     */
    atm_uint_t          __overflow;
    /*
     * When the log data is too small to invoke an
     * overflow, the logger thread should be waked
     * up until a timeout.
     */
    atm_uint_t          __secondTimeout;
    /*
     * The procuder push data into __currrent only,
     * when the __current is not available for new
     * data to come in, the producer move all Buff
     * ers into __alternate in appending manner, t
     * hen the producer wake up the logger thread.
     */
    buffs_t             __current;
    // __alternate is an infinite buffer.
    buffs_t             __alternate;
    /*
     * __loggerLocal is private to the logger thre
     * ad, so it doesn't need protection.
     */
    buffs_t             __loggerLocal;
    /* log handler */
    std::ofstream       __out;
    std::string         __logFile;
    /* running status */
    atm_bool_t          __running;
    tid_t               __threadID;
    /* 
     * synchronization variables
     * 1. __lockLogFile use to protect __logFile & __out, mainly for 
     *    flush2LogFile and setLogFile.
     * 2. __lockBuff used to protect __current & __alternate.
     * 3. __wait4Flush makes the logger sleep when __current isn't o
     *    verflow or the __secondTimeout hasn't reached yet.
     */
    file_mtx_t          __lockLogFile;
    buff_mtx_t          __lockBuff;
    cond_t              __wait4Flush;
};
