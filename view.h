#pragma once

#include "allocator.h"
#include "base.h"
#include "memory.h"

#ifdef STLEMM_DEBUG
#include <cstdio>
#include <type_traits>
#endif

template <typename T>
class View
{
public:
    explicit View(T* ptr, const usize count) : _ptr(ptr), _item_count(count) {}
    constexpr T* data() const { return _ptr; }
    constexpr usize count() const { return _item_count; }
    constexpr usize size() const { return _item_count * sizeof(T); }

    template <typename AllocType>
    constexpr View<std::remove_cv_t<T>> copy(Allocator::Base<AllocType>& alloc)
    {
        T* new_memory = alloc.alloc(this->size());
        Memory::copy(new_memory, this->_ptr, this->size());
        return View<std::remove_cv_t<T>>(new_memory, this->size());
    }

    constexpr View<std::remove_cv_t<T>> copy()
    {
        T* new_memory = Memory::alloc<T>(this->count());
        Memory::copy(new_memory, this->_ptr, this->size());
        return View<std::remove_cv_t<T>>(new_memory, this->size());
    }

#ifdef STLEMM_DEBUG
    void dump()
    {
        printf("--- View debug info: ---\n");
        printf("View address\t: %p\nItem count\t: %zu\n", _ptr, _item_count);
        if constexpr (std::is_same_v<std::remove_cv_t<T>, char>)
        {
            printf("Str data\t: %.*s\n", (i32)_item_count, _ptr);
        }
    }
#endif

private:
    T* _ptr;
    const usize _item_count;
};

inline View<const char> operator""_View(const char* ptr, std::size_t size)
{
    return View<const char>{ptr, size};
}
