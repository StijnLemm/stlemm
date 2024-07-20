#pragma once

#include "base.h"

namespace Memory
{

void copy_void(void* dest, const void* src, const usize size) noexcept;

template <typename T>
static constexpr void copy(T* dest, const T* src, const usize count) noexcept
{
    constexpr usize transfer_size = sizeof(usize);
    copy_void((void*)dest, (void*)src, sizeof(T) * count);
}

template <typename T>
static constexpr T&& move(T& v) noexcept
{
    return static_cast<T&&>(v);
}

class Heap
{
public:
    template <typename T>
    static constexpr T* alloc(const usize count) noexcept
    {
        return (T*)alloc_sz(sizeof(T) * count);
    }

    template <typename T>
    static constexpr T* realloc(T* ptr, const usize count) noexcept
    {
        return (T*)realloc_sz(ptr, sizeof(T) * count);
    }

    template <typename T>
    static constexpr void free(T* ptr) noexcept
    {
        if (ptr == nullptr) return;

        free_void((void*)ptr);
    }

    static void* start();

private:
    static void* alloc_sz(const usize size) noexcept;
    static void* realloc_sz(void* ptr, const usize new_size) noexcept;
    static void free_void(void* ptr) noexcept;

    friend class HeapGuard;
};

}  // namespace Memory

// Define our own placement new operator
inline void* operator new(usize size, void* ptr) noexcept
{
    return ptr;
}
