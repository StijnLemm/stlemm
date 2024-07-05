#pragma once

#include "base.h"

#ifdef STLEMM_DEBUG
#include <cstdio>
#include <type_traits>
#endif

template <typename T>
class View
{
public:
    explicit View(T* ptr, const usize count) : _ptr(ptr), _item_count(count) {}

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
