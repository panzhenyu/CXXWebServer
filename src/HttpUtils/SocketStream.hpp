#pragma once

#include <streambuf>
#include <istream>
#include <ostream>
#include <memory>

/*
 * SocketStreamBuffer
 * Mainly provide helper function overflow and underflow for stream object.
 * This buffer doesn't shutdown the socket.
 */
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
    virtual int_type overflow(int_type) override;
    int sync();

    // used by reader
    virtual int_type underflow() override;
private:
    char        __buffer[BFSIZE];
    socket_t    __socket;
};

/*
 * SocketInputStream
 * A simple wrapper for istream to read data from socket.
 * Destroy SocketStreamBuffer object automatically.
 */
class SocketInputStream: public std::istream {
public:
    using ssbuff_uptr_t = std::shared_ptr<SocketStreamBuffer>;
    SocketInputStream(ssbuff_uptr_t);
    SocketInputStream(const SocketInputStream&) = delete;
    ~SocketInputStream() = default;
private:
    ssbuff_uptr_t __buffer;
};

/*
 * SocketInputStream
 * A simple wrapper for ostream to write data into socket.
 * Destroy SocketStreamBuffer object automatically.
 */
class SocketOutputStream: public std::ostream {
public:
    using ssbuff_uptr_t = std::shared_ptr<SocketStreamBuffer>;
    SocketOutputStream(ssbuff_uptr_t);
    SocketOutputStream(const SocketOutputStream&) = delete;
    ~SocketOutputStream() = default;
private:
    ssbuff_uptr_t __buffer;
};
