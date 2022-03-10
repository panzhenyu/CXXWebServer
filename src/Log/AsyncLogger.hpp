#pragma once

#include <mutex>
#include <thread>
#include <atomic>
#include <memory>
#include <fstream>
#include <condition_variable>
#include "Buffer.hpp"
#include "../Error.hpp"

/*
 * AsyncLogger
 * Performed in a singleton pattern.
 * AsyncLogger will be accessed by many threads, so it must enable thread-safe interfaces.
 */
class AsyncLogger {
public:
    using atm_uint_t    = std::atomic<unsigned>;
    using atm_bool_t    = std::atomic<bool>;
    using ch_buff_t     = Buffer<char>;
    using buffs_t       = std::vector<ch_buff_t>;
    using cond_t        = std::condition_variable;
    using tid_t         = std::thread::id;
public:
    static constexpr unsigned DEFAULT_OVERFLOW = 2;
    static AsyncLogger& getAsyncLogger();
protected:
    AsyncLogger();
    AsyncLogger(const AsyncLogger&) = delete;

    void run();
    bool currentAvailable4(ch_buff_t& _buff);
    void append2Current(ch_buff_t&&);
    server_err_t flushAlternate2LogFile();
public:
    ~AsyncLogger();

    void stop();
    bool isRunning();
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
    buffs_t             __current;
    buffs_t             __alternate;
    std::ofstream       __out;
    std::string         __logFile;
    /* running status */
    atm_bool_t          __running;
    tid_t               __threadID;
    /*
     * Double Buffer synchronization mechanism
     * 1. __lockCurrent provides atomic access for __current.
     * 2. __lockAlternate provides atomic access for __alternate.
     * 3. __waitAlternateWritable is used to synchronize AsyncLogger thread(the consumer). 
     * 4. __waitAlternateWriteDone is used to synchronize other thread(the producer).
     * As for consumer:
     *    1. lock __lockAlternate.
     *    2. wait __waitAlternateWritable for some time.
     *    3. wait returns hints a timeout or a writeable __alternate.
     *    4. if wait returns due to timeout, a consumer should check
     *       whether __alternate is writeable, otherwise try to swap
     *       __alternate and __current. Note that if swap failed for 
     *       lock ownership , the swap will be handled by a producer
     *       in future.
     *    5. 
     */
    std::mutex          __lockCurrent;
    std::mutex          __lockAlternate;
    cond_t              __waitAlternateWritable;
    cond_t              __waitAlternateWriteDone;
};
