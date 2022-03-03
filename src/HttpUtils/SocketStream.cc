#include <netinet/in.h>
#include "SocketStream.hpp"

SocketStreamBuffer::SocketStreamBuffer(socket_t _socket): __socket(_socket) {
    // set for input
    setg(__buffer, __buffer, __buffer);
    // set for output
    setp(__buffer, __buffer+BFSIZE-1);
}

SocketStreamBuffer::~SocketStreamBuffer() { sync(); }

// used by writer
int SocketStreamBuffer::flush() {
    int total, rest, tmp;
    rest = total = pptr() - pbase();
    while (rest) {
        if (0 >= (tmp=send(__socket, __buffer, rest, 0))) break;
        rest -= tmp;
    }
    pbump(rest-total);
    if (rest) return EOF;
    return total;
}

SocketStreamBuffer::int_type SocketStreamBuffer::overflow(int_type _c) {
    if (_c != EOF) {
        *pptr() = _c;
        pbump(1);
    }
    if (EOF == flush()) return EOF;
    return _c;
}

int SocketStreamBuffer::sync() {
    if (EOF == flush()) return EOF;
    return 0;
}

// used by reader
SocketStreamBuffer::int_type SocketStreamBuffer::underflow() {
    int recvLen;

    if (0 >= (recvLen=recv(__socket, __buffer, BFSIZE, 0))) return EOF;
    setg(__buffer, __buffer, __buffer+recvLen);

    return *gptr();
}

SocketInputStream::SocketInputStream(ssbuff_sptr_t _buff):
    std::istream(_buff.get()), __buffer(_buff) {}

SocketOutputStream::SocketOutputStream(ssbuff_sptr_t _buff):
    std::ostream(_buff.get()), __buffer(_buff)  {}
