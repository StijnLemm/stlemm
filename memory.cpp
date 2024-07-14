#include "memory.h"

#include <unistd.h>

#include "base.h"

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

static constexpr usize CHUNK_IN_USE_FLAG = ((usize)1 << ((sizeof(usize) * 8) - 1));
static constexpr usize MAX_CHUNK_SIZE = CHUNK_IN_USE_FLAG - 1;

struct MemoryChunk
{
    // MemoryChunk* prev = nullptr;
    // MemoryChunk* next = nullptr;
    // usize in_use = 0;
    usize size = 0;

    constexpr usize chunk_size() const
    {
        return this->size & ~CHUNK_IN_USE_FLAG;
    }

    constexpr void set_chunk_size(const usize size)
    {
        if (is_used())
        {
            this->size = size + CHUNK_IN_USE_FLAG;
        }
        else
        {
            this->size = size;
        }
    }

    constexpr usize combined_chunk_size(const MemoryChunk* other) const
    {
        return other->chunk_size() + this->chunk_size() + sizeof(MemoryChunk);
    }

    constexpr void combine_into_me(const MemoryChunk* other)
    {
        this->set_chunk_size(combined_chunk_size(other));
    }

    constexpr bool is_used() const
    {
        return this->size & CHUNK_IN_USE_FLAG;
    }

    constexpr void set_used(const bool used)
    {
        if (used)
        {
            this->size |= CHUNK_IN_USE_FLAG;
        }
        else
        {
            this->size &= ~CHUNK_IN_USE_FLAG;
        }
    }

    constexpr MemoryChunk* next_chunk()
    {
        return (MemoryChunk*)((this->data_ptr<u8>()) + this->chunk_size());
    }

    template <typename T>
    constexpr T* data_ptr()
    {
        return reinterpret_cast<T*>(this + 1);
    }
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
    chunk_ptr->set_used(false);
}

static constexpr usize calculate_aligned_size(const usize size)
{
    // add alignment for speed
    return sizeof(usize) + (((size - 1) / sizeof(usize)) * sizeof(usize));
}

void* Memory::Heap::alloc_sz(const usize size) noexcept
{
    const usize needed_bytes = calculate_aligned_size(size);
    MemoryChunk* iter = memoryChunks;
    MemoryChunk* prev = nullptr;

    while (iter->chunk_size() != 0 && (iter->is_used() || iter->chunk_size() < needed_bytes))
    {
        if (!iter->is_used() && prev != nullptr && !prev->is_used() &&
            prev->combined_chunk_size(iter) >= needed_bytes)
        {
            prev->combine_into_me(iter);
            prev->set_used(true);
            return prev->data_ptr<void>();
        }

        prev = iter;
        iter = iter->next_chunk();
    }

    if (iter->chunk_size() == 0)
    {
        sbrk(needed_bytes + sizeof(MemoryChunk));

        // set next to tail.
        *(iter->next_chunk()) = MemoryChunk{
            0,  // size
        };

        // set size of chunk
        iter->set_chunk_size(needed_bytes);
        iter->set_used(true);
        return iter->data_ptr<void>();
    }

    iter->set_used(true);
    return iter->data_ptr<void>();
}
