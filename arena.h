#pragma once

#include "base.h"
namespace Allocator
{
class Arena
{
private:
    explicit Arena(u8* ptr, const usize cap) : memory(ptr), cap(cap) {}
    u8* memory;
    const usize cap;

    usize position = 0;
};
}  // namespace Allocator
