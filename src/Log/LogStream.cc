#include "LogStream.hpp"
#include "AsyncLogger.hpp"

void LogStreamBuffer::flush() { __asyncLogger.append(std::move(__buffer)); }

LogStreamBuffer::LogStreamBuffer(AsyncLogger& _asyncLogger): __asyncLogger(_asyncLogger) {
    setp(__buffer.begin(), __buffer.end()-1);
}

LogStreamBuffer::~LogStreamBuffer() { flush(); }

LogStreamBuffer::int_type 
LogStreamBuffer::overflow(int_type _ch) {
    if (EOF != _ch) {
        *pptr() = _ch;
        pbump(1);
    }
    return EOF;
}

LogStream::LogStream(lsbuff_uptr_t _pBuffer): 
std::ostream(_pBuffer.get()), __buffer(_pBuffer) {}
