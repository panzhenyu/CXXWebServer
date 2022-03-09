#pragma once

#include <memory>
#include <fstream>
#include "Buffer.hpp"

/*
 * Singleton
 */
class AsyncLogger {
public:
    using writer_sptr_t = std::shared_ptr<std::ofstream>;
private:
    AsyncLogger();
    AsyncLogger(const AsyncLogger&) = delete;
    void run();
    void flush();
public:
    ~AsyncLogger();
    void append(Buffer&& b);
private:
    Buffer          __buffers[4];
    writer_sptr_t   __out;
};
