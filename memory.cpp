#include "memory.h"

#include <unistd.h>

namespace Memory
{

void copy_void(void* dest, const void* src, const usize size) noexcept
{
    constexpr usize transfer_size = sizeof(usize);

    usize* dest_iter_uz = (usize*)dest;
    usize* src_iter_uz = (usize*)src;
    const usize* dest_ter_goal_uz = dest_iter_uz + (size / transfer_size);
    for (; dest_iter_uz < dest_ter_goal_uz; dest_iter_uz++)
    {
        *dest_iter_uz = *src_iter_uz;
        src_iter_uz++;
    }

    // this can be even more efficient by trying 32 and 16 bit transfers.
    u8* dest_iter_c = (u8*)dest_iter_uz;
    u8* src_iter_c = (u8*)src_iter_uz;
    const u8* dest_iter_goal_c = dest_iter_c + (size % transfer_size);
    for (; dest_iter_c < dest_iter_goal_c; dest_iter_c++)
    {
        *dest_iter_c = *src_iter_c;
        src_iter_c++;
    }
}

}  // namespace Memory

struct MemoryChunk
{
    MemoryChunk* prev = nullptr;
    MemoryChunk* next = nullptr;
    usize in_use = 0;
    usize size = 0;
};

static MemoryChunk* memoryChunks = (MemoryChunk*)sbrk(sizeof(MemoryChunk));
static void* _start = (memoryChunks);

void* Memory::Heap::start()
{
    return _start;
}

void Memory::Heap::free_void(void* ptr) noexcept
{
    MemoryChunk* chunk_ptr = (MemoryChunk*)ptr;
    chunk_ptr--;
    chunk_ptr->in_use = 0;
}

static constexpr usize calculate_aligned_size(const usize size)
{
    // add allignment of chunk header size
    return sizeof(MemoryChunk) + (((size - 1) / sizeof(MemoryChunk)) * sizeof(MemoryChunk));
}

void* Memory::Heap::alloc_sz(const usize size) noexcept
{
    const usize needed_bytes = calculate_aligned_size(size);
    MemoryChunk* iter = memoryChunks;
    while (iter->next != nullptr && (iter->in_use == 1 || iter->size < needed_bytes))
    {
        if (iter->in_use == 0 && iter->prev != nullptr && iter->prev->in_use == 0 &&
            iter->prev->size + iter->size + sizeof(MemoryChunk) >= needed_bytes)
        {
            // combine the two memory chunks
            iter->next->prev = iter->prev;
            iter->prev->next = iter->next;
            iter->prev->size = iter->prev->size + iter->size + sizeof(MemoryChunk);
            return (void*)(iter->prev + 1);
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
        return (void*)(iter + 1);
    }

    iter->in_use = 1;
    return (void*)(iter + 1);
}
