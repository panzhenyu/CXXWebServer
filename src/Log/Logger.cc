#include <ctime>
#include <cstring>
#include "Logger.hpp"
#include "LogStream.hpp"

Logger::Logger(const std::string& _file, int _line, stream_sptr_t _pStream): 
__file(__FILE__), __line(__LINE__), __pStream(_pStream) { logBeginTag(); }

Logger::~Logger() { logEndTag(); }

std::ostream& Logger::getLogStream() { return *__pStream; }

// NOTE: There is no need free the return pointer of local time
void Logger::logCurrentTime() {
    tm *info;
    char *timestr;
    time_t rawtime;

    time(&rawtime);
    info = localtime(&rawtime);
    if (info && (timestr=asctime(info))) {
        timestr[std::strlen(timestr)-1] = '\0';
        getLogStream() << '[' << timestr << ']';
    } else getLogStream() << "[??]";
}

void Logger::logBeginTag() {
    // log begin time
    logCurrentTime();
    getLogStream() << " Logged by: " << __file << ": " << __line << '.' << std::endl;
}

void Logger::logEndTag() {
    getLogStream() << '\n';
    logCurrentTime();
    getLogStream() << " Logged end." << std::endl;
}
