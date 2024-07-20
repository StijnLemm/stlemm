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

    constexpr bool is_tail()
    {
        return chunk_size() == 0;
    }

    constexpr bool would_fit(const usize size)
    {
        return chunk_size() >= size;
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

    static MemoryChunk* from_data_ptr(void* ptr)
    {
        return ascast(ptr, MemoryChunk*) - 1;
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
    MemoryChunk* chunk_ptr = MemoryChunk::from_data_ptr(ptr);
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

    while (!iter->is_tail() && (iter->is_used() || iter->chunk_size() < needed_bytes))
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

    if (iter->is_tail())
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

// TODO: if the new_size < chunk_ptr->chunk_size() then we need to make it smaller and create a new
// chunk?
void* Memory::Heap::realloc_sz(void* ptr, const usize new_size) noexcept
{
    MemoryChunk* chunk_ptr = MemoryChunk::from_data_ptr(ptr);

    // check if the chunk is big enough for resize.
    if (chunk_ptr->chunk_size() >= new_size)
    {
        return ptr;
    }

    // check if the next chunk is not in use and big enough to merge.
    // NOTE: we could check the next chunk again if the next is only not big enough to merge more
    // chunks into one.
    MemoryChunk* iter = chunk_ptr->next_chunk();

    MemoryChunk accumulator_chunk(chunk_ptr->chunk_size());
    while (!iter->is_tail() && !iter->is_used())
    {
        // add their chunk size to the combine attempt
        accumulator_chunk.combine_into_me(iter);

        if (accumulator_chunk.would_fit(new_size))
        {
            chunk_ptr->set_chunk_size(accumulator_chunk.chunk_size());
            return ptr;
        }

        iter = iter->next_chunk();
    }

    // if iter is tail, we could resize heap
    if (iter->is_tail())
    {
        // add ptr allignment, the chunk size is allready the at minimum the size of a minimum
        // chunk.
        const usize alligned_size = calculate_aligned_size(new_size);

        // allocate the remaining space WITH the padded minimum chunk size.
        sbrk(alligned_size - accumulator_chunk.chunk_size());

        chunk_ptr->set_chunk_size(alligned_size);

        // set next to tail.
        *(chunk_ptr->next_chunk()) = MemoryChunk::tail();
        return ptr;
    }

    // if iter is used we need to reallocate the chunk somewhere else and copy. Ouch!
    free(ptr);
    auto new_mem = alloc_sz(new_size);
    copy_void(new_mem, ptr, new_size);
    return new_mem;
}
