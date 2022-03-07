#pragma once

#include <functional>

template <typename _Key, 
    typename _Val, 
    typename _Hash=std::hash<_Key>, 
    typename _Pred=std::equal_to<_Key>>
struct ICache {
    using key_t = _Key;
    using val_t = _Val;
protected:
    ICache() = default;
    ICache(const ICache&) = delete;
    virtual ~ICache() = default;
public:
    virtual std::pair<bool, val_t> get(const key_t&) = 0;
    virtual bool put(const key_t&, const val_t&) = 0;
};
