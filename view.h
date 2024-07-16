#pragma once

#include "allocator.h"
#include "base.h"
#include "memory.h"
#include "owner.h"

#ifdef STLEMM_DEBUG
#include <cstdio>
#include <type_traits>
#endif

template <typename T>
class View
{
public:
    View(T* ptr, const usize count) : _ptr(ptr), _item_count(count) {}
    constexpr T* data() const
    {
        return _ptr;
    }

    constexpr usize count() const
    {
        return _item_count;
    }

    constexpr usize size() const
    {
        return _item_count * sizeof(T);
    }

    T* begin()
    {
        return _ptr;
    }

    T* end()
    {
        return _ptr + count();
    }

    Memory::Owner<char> c_str() const
    {
        auto ret = Memory::Owner<char>(Memory::Heap::alloc<char>(count() + 1));
        Memory::copy(ret.get(), this->data(), count());
        ret.get()[count()] = '\0';
        return Memory::move(ret);
    }

#ifdef STLEMM_DEBUG
    void dump() const
    {
        printf("--- View debug info: ---\n");
        printf("View address\t: %p\nItem count\t: %zu\n", _ptr, _item_count);
        if constexpr (std::is_same_v<std::remove_cv_t<T>, char>)
        {
            printf("Str data\t:\n%.*s\n", (i32)_item_count, _ptr);
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
