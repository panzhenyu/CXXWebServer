#pragma once

#include <streambuf>
#include <istream>
#include <ostream>
#include <memory>

class SocketStreamBuffer: public std::streambuf {
public:
    using socket_t = int;
    static constexpr int BFSIZE = 4096;
private:
    int flush();
public:
    SocketStreamBuffer(socket_t);
    SocketStreamBuffer(const SocketStreamBuffer&) = delete;
    ~SocketStreamBuffer();

    // used by writer
    int_type overflow(int_type);
    int sync();

    // used by reader
    int_type underflow();
private:
    char        __buffer[BFSIZE];
    socket_t    __socket;
};

class SocketInputStream: public std::istream {
public:
    using ssbuff_sptr_t = std::shared_ptr<SocketStreamBuffer>;
    SocketInputStream(ssbuff_sptr_t);
    SocketInputStream(const SocketInputStream&) = delete;
    ~SocketInputStream() = default;
    ssbuff_sptr_t __buffer;
};

class SocketOutputStream: public std::ostream {
public:
    using ssbuff_sptr_t = std::shared_ptr<SocketStreamBuffer>;
    SocketOutputStream(ssbuff_sptr_t);
    SocketOutputStream(const SocketOutputStream&) = delete;
    ~SocketOutputStream() = default;
private:
    ssbuff_sptr_t __buffer;
};
