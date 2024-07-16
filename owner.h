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

    explicit Owner(T* ptr) : _ptr(ptr) {}
    explicit Owner() : _ptr(nullptr) {}

    ~Owner() noexcept
    {
        if (this->_ptr != nullptr)
        {
            this->_ptr->~T();
        }
        Memory::Heap::free(this->_ptr);
    }

    Owner(Owner<T>& other) = delete;
    Owner(const Owner<T>& other) = delete;

    Owner(Owner<T>&& other) noexcept
    {
        this->_ptr = other._ptr;
        other.reset();
    }
    Owner(const Owner<T>&& other) = delete;

    Owner& operator=(Owner<T>&& other)
    {
        this->_ptr = other._ptr;
        other.reset();
        return *this;
    }
    Owner operator=(const Owner<T>&& other) = delete;

    void reset()
    {
        _ptr = nullptr;
    }

    T* get() const
    {
        return _ptr;
    }

private:
    T* _ptr;
};

}  // namespace Memory
