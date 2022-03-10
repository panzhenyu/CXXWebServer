#pragma once

#include <memory>
#include <ostream>
#include "Buffer.hpp"

class AsyncLogger;

class LogStreamBuffer: public std::streambuf {
public:
    using ch_buff_t = Buffer<char>;
private:
    // Flush all data from buffer to asyncLogger.
    void flush();
public:
    LogStreamBuffer(AsyncLogger&);
    LogStreamBuffer(const LogStreamBuffer&) = delete;
    ~LogStreamBuffer();
    // overflow occuring hints the __buffer is full, return EOF directly.
    virtual int_type overflow(int_type) override;
private:
    ch_buff_t       __buffer;
    AsyncLogger&    __asyncLogger;
};

/*
 * LogStream
 * Used to buffer log data into a LogStreamBuffer object.
 * All flush operations are dominated by LogStreamBuffer.
 */
class LogStream: public std::ostream {
public:
    using lsbuff_uptr_t = std::shared_ptr<LogStreamBuffer>;
public:
    LogStream(lsbuff_uptr_t);
    LogStream(const LogStream&) = delete;
    ~LogStream() = default;
private:
    lsbuff_uptr_t   __buffer;
};
