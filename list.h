#pragma once

#include "memory.h"
#include "owner.h"
#include "view.h"

namespace Memory
{

template <typename T>
class List : public Owner<T>
{
public:
    List(T* ptr, const usize size) : Owner<T>(ptr), _size(size) {}

    static List<T> create(const usize initial_size)
    {
        auto mem = Memory::Heap::alloc<T>(initial_size);
        return List<T>(mem, initial_size);
    }

    T& operator[](const usize index)
    {
        return this->get()[index];
    }

    const T& operator[](const usize index) const
    {
        return this->get()[index];
    }

    constexpr View<T> as_view()
    {
        return View<T>(this->get(), _size);
    }

private:
    const usize _size;
};

}  // namespace Memory
