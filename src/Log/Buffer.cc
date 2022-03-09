#include <algorithm>
#include "Buffer.hpp"

Buffer::Buffer(): __chBuffer(BFSIZE) {}

unsigned Buffer::append(const std::string& _s) {
    unsigned total;

    total = std::min<size_t>(_s.length(), BFSIZE-__cur);
    std::copy(_s.data(), _s.data()+total, __chBuffer.begin()+__cur);
    __cur += total;

    return total;
}
