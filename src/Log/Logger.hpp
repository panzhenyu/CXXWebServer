#pragma once

#include <mutex>
#include <memory>
#include <string>
#include "LogStream.hpp"
#include "AsyncLogger.hpp"

/*
 * Logger
 * Provied interfaces to access log file.
 * Member __logFile may be modified by many thread(although they shouldn't do this), 
 * so the modification of __logFile need to be thread-safe.
 */
class Logger {
public:
    using stream_sptr_t = std::shared_ptr<std::ostream>;
public:
    Logger(const std::string&, int, stream_sptr_t);
    Logger(const Logger&) = delete;
    ~Logger();
    std::ostream& getLogStream();
private:
    void logCurrentTime();
    void logBeginTag();
    void logEndTag();
private:
    std::string         __file;
    int                 __line;
    stream_sptr_t       __pStream;
};

#define LOG2DIARY   (Logger(__FILE__, __LINE__, \
    std::make_shared<LogStream>(                \
        std::make_shared<LogStreamBuffer>(      \
            AsyncLogger::getAsyncLogger()))).getStream())
