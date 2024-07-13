#pragma once

#include <new>  // fuck this shit

#include "memory.h"

namespace Memory
{

template <typename T>
class Owner
{
public:
    template <typename... Args>
    static Owner<T> create(Args... args) noexcept
    {
        T* mem = Heap::alloc<T>(1);
        return Owner<T>(new (mem) T{args...});
    }

    Owner(T* ptr) : _ptr(ptr) {}
    ~Owner() noexcept
    {
        Heap::free(_ptr);
    }

    Owner(Owner<T>& other) = delete;
    Owner(const Owner<T>& other) = delete;

    Owner(Owner<T>&& other) noexcept
    {
        _ptr = other._ptr;
        other._ptr = nullptr;
    }

    Owner(const Owner<T>&& other) = delete;

    T* get()
    {
        return _ptr;
    }

private:
    T* _ptr;
};

}  // namespace Memory
