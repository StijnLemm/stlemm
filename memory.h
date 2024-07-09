#pragma once

#include <cstdio>

#include "base.h"

namespace Memory
{
template <typename T>
static constexpr void copy(T* dest, const T* src, const usize count)
{
    constexpr usize transfer_size = sizeof(usize);

    usize* dest_iter_uz = (usize*)dest;
    usize* src_iter_uz = (usize*)src;
    const usize* dest_ter_goal_uz = dest_iter_uz + ((count * sizeof(T)) / transfer_size);
    for (; dest_iter_uz < dest_ter_goal_uz; dest_iter_uz++)
    {
        *dest_iter_uz = *src_iter_uz;
        src_iter_uz++;
    }

    // this can be even more efficient by trying 32 and 16 bit transfers.
    u8* dest_iter_c = (u8*)dest_iter_uz;
    u8* src_iter_c = (u8*)src_iter_uz;
    const u8* dest_iter_goal_c = dest_iter_c + ((count * sizeof(T)) % transfer_size);
    for (; dest_iter_c < dest_iter_goal_c; dest_iter_c++)
    {
        *dest_iter_c = *src_iter_c;
        src_iter_c++;
    }
}

// TODO: do we want, type count and calc this here or byte size casted into type?
template <typename T>
static constexpr T* alloc(const usize count)
{
    return nullptr;
}
}  // namespace Memory
