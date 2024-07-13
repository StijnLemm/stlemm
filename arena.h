#pragma once

#include "base.h"

namespace Allocator
{

template <typename T>
struct Base
{
    virtual ~Base() = default;
    void* alloc(const usize size_bytes)
    {
        return static_cast<T*>(this)->crtp_alloc(size_bytes);
    }
};

class Arena : public Base<Arena>
{
public:
    static constexpr usize DEFAULT_INFLATION_CAP = 1024u;
    static Arena inflate(const usize cap = DEFAULT_INFLATION_CAP)
    {
        return Arena(nullptr, cap);
    }

private:
    void* alloc(const usize size_bytes)
    {
        return nullptr;
    }

    explicit Arena(u8* ptr, const usize cap) : memory(ptr), cap(cap) {}
    u8* memory;
    const usize cap;

    usize position = 0;

    friend Base<Arena>;
};
}  // namespace Allocator
