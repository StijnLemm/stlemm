#include "memory.h"

#include <unistd.h>

#include "base.h"

namespace Memory
{

void copy_void(void* dest, const void* src, const usize size) noexcept
{
    constexpr usize transfer_size = sizeof(usize);

    usize* dest_iter_uz = ascast(dest, usize*);
    const usize* src_iter_uz = ascast(src, const usize*);
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

static constexpr usize CHUNK_IN_USE_FLAG = (ascast(1, usize) << ((sizeof(usize) * 8) - 1));
static constexpr usize MAX_CHUNK_SIZE = CHUNK_IN_USE_FLAG - 1;

class MemoryChunk
{
public:
    constexpr MemoryChunk(const usize size) : _size(size) {}

    static MemoryChunk tail()
    {
        return MemoryChunk(0);
    }

    constexpr usize chunk_size() const
    {
        return this->_size & ~CHUNK_IN_USE_FLAG;
    }

    constexpr void set_chunk_size(const usize size)
    {
        if (is_used())
        {
            this->_size = size + CHUNK_IN_USE_FLAG;
        }
        else
        {
            this->_size = size;
        }
    }

    constexpr bool is_used() const
    {
        return this->_size & CHUNK_IN_USE_FLAG;
    }

    constexpr void set_used(const bool used)
    {
        if (used)
        {
            this->_size |= CHUNK_IN_USE_FLAG;
        }
        else
        {
            this->_size &= ~CHUNK_IN_USE_FLAG;
        }
    }

    constexpr MemoryChunk* next_chunk()
    {
        return (MemoryChunk*)((this->data_ptr_as<u8>()) + this->chunk_size());
    }

    constexpr usize combined_chunk_size(const MemoryChunk* other) const
    {
        return other->chunk_size() + this->chunk_size() + sizeof(MemoryChunk);
    }

    constexpr void combine_into_me(const MemoryChunk* other)
    {
        this->set_chunk_size(combined_chunk_size(other));
    }

    template <typename T>
    constexpr T* data_ptr_as()
    {
        return (T*)(this + 1);
    }

private:
    usize _size = 0;
};

static constexpr usize MIN_CHUNK_SIZE = 64u;
static MemoryChunk* memoryChunks = (MemoryChunk*)sbrk(sizeof(MemoryChunk));
static void* _start = (memoryChunks);

void* Memory::Heap::start()
{
    return _start;
}

void Memory::Heap::free_void(void* ptr) noexcept
{
    MemoryChunk* chunk_ptr = ascast(ptr, MemoryChunk*);
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
            return prev->data_ptr_as<void>();
        }

        prev = iter;
        iter = iter->next_chunk();
    }

    if (iter->chunk_size() == 0)
    {
        const usize new_chunk_size =
            (needed_bytes > MIN_CHUNK_SIZE) ? needed_bytes : MIN_CHUNK_SIZE;
        const usize allocating_space = new_chunk_size + sizeof(MemoryChunk);

        sbrk(allocating_space);

        // set size of chunk and used
        iter->set_chunk_size(new_chunk_size);
        iter->set_used(true);

        // set next to tail.
        *(iter->next_chunk()) = MemoryChunk::tail();

        return iter->data_ptr_as<void>();
    }

    iter->set_used(true);
    return iter->data_ptr_as<void>();
}
