#pragma once

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
        T* mem = Memory::Heap::alloc<T>(1);
        return Owner<T>(new (mem) T{args...});
    }

    Owner(T* ptr) : _ptr(ptr) {}
    ~Owner() noexcept
    {
        if (_ptr != nullptr)
        {
            _ptr->~T();
        }
        Memory::Heap::free(_ptr);
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

// template <typename T>
// class OwnerView : private Owner<T>
// {
// public:
//     constexpr View<T> as_view()
//     {
//         return View<T>(get(), size);
//     }
//
// private:
//     const usize size;
// };

}  // namespace Memory
