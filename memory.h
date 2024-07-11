#pragma once

#include <unistd.h>

#include "base.h"
#include "heap_guard.h"

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

// ---------------- ALLOC ---------------------
struct MemoryChunk
{
    MemoryChunk* prev = nullptr;
    MemoryChunk* next = nullptr;
    usize in_use = 0;
    usize size = 0;
};

class Heap
{
public:
    static inline MemoryChunk* memoryChunks = (MemoryChunk*)sbrk(sizeof(MemoryChunk));
    static inline void* start = (memoryChunks);
};

template <typename T>
static constexpr T* alloc(const usize count)
{
    // add allignment of ptr size
    const usize needed_bytes =
        sizeof(usize) + ((((sizeof(T) * count) - 1) / sizeof(usize)) * sizeof(usize));

    MemoryChunk* iter = Heap::memoryChunks;
    while (iter->next != nullptr && (iter->in_use == 1 || iter->size < needed_bytes))
    {
        if (iter->in_use == 0 && iter->prev != nullptr && iter->prev->in_use == 0 &&
            iter->prev->size + iter->size + sizeof(MemoryChunk) >= needed_bytes)
        {
            // combine the two memory chunks
            iter->next->prev = iter->prev;
            iter->prev->next = iter->next;
            iter->prev->size = iter->prev->size + iter->size + sizeof(MemoryChunk);
            return (T*)(iter->prev + 1);
        }
        iter = iter->next;
    }

    if (iter->next == nullptr)
    {
        sbrk(needed_bytes + sizeof(MemoryChunk));

        // set next to tail.
        iter->next = (MemoryChunk*)((u8*)(iter + 1) + needed_bytes);
        *(iter->next) = MemoryChunk{
            iter,     // prev
            nullptr,  // next
            0,
            0,  // size
        };

        // set size of chunk
        iter->size = needed_bytes;
        iter->in_use = 1;
        return (T*)(iter + 1);
    }

    iter->in_use = 1;
    return (T*)(iter + 1);
}

template <typename T>
static void free(T* ptr)
{
    MemoryChunk* chunk_ptr = (MemoryChunk*)ptr;
    chunk_ptr--;
    chunk_ptr->in_use = 0;
}

}  // namespace Memory
