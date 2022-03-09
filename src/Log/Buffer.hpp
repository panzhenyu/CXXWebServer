#pragma once

#include <vector>
#include <string>

/*
 * Buffer
 * Not a thread-safe class.
 * Use vector<char> to buffer a fixed size characters.
 * Enable move construction & assign operator to accelerate data transfor.
 */
class Buffer {
public:
    using container_t = std::vector<char>;
    static constexpr unsigned BFSIZE = 4096;
public:
    Buffer();
    Buffer(const Buffer&) = delete;
    ~Buffer() = default;
    Buffer(Buffer&&) = default;
    Buffer& operator=(Buffer&&) = default;

    unsigned append(const std::string&);
    unsigned getLength();
private:
    /*
     * __cur hints the number of characters already buffered in __chBuffer.
     * __cur also represent the location of next character to be written.
     */
    unsigned        __cur;
    container_t     __chBuffer;
};
