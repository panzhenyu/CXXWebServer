#pragma once

#include <vector>

/*
 * Buffer
 * Buffer with fixed size, no thread-safe guarantee.
 * Enable move construction & move assign operator to accelerate data transfor.
 */
template <typename _Tp>
class Buffer {
public:
    using value_t       = _Tp;
    using pointer_t     = _Tp*;
    using container_t   = std::vector<value_t>;
    static constexpr unsigned DEFAULT_BFSIZE = 4096;
public:
    Buffer(unsigned _size=DEFAULT_BFSIZE): __chBuffer(_size), __cur(&__chBuffer[0]) {}

    Buffer(const Buffer&) = delete;

    ~Buffer() = default;

    Buffer(Buffer&&) = default;
    
    Buffer& operator=(Buffer&&) = default;

    bool empty() { return __cur == begin(); }

    void clear() { __cur = begin(); }

    unsigned length() { return __cur - begin(); }

    unsigned capacity() { return __chBuffer.size(); }

    unsigned rest() { return capacity() - length(); }

    pointer_t begin() { return &__chBuffer[0]; }

    pointer_t cur() { return __cur; }

    pointer_t end() { return begin() + capacity(); }

    template <typename _InputIterator>
    unsigned append(_InputIterator _begin, _InputIterator _end) {
        unsigned total;
        pointer_t tail;
        for (total=0, tail=end(); _begin<_end && __cur<tail; ++__cur, ++_begin, ++total) *__cur = *_begin;
        return total;
    }
private:
    /*
     * __cur hints the number of characters already buffered in __chBuffer.
     * __cur also represent the location of next character to be written.
     */
    container_t     __chBuffer;
    pointer_t       __cur;
};
