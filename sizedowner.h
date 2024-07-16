#pragma once

#include "memory.h"
#include "owner.h"
#include "view.h"

namespace Memory
{

template <typename T>
class ViewOwner : public Owner<T>
{
public:
    ViewOwner(T* ptr, const usize size) : Owner<T>(ptr), _size(size) {}

    static ViewOwner<T> create(const usize size)
    {
        auto mem = Memory::Heap::alloc<T>(size);
        return ViewOwner<T>(mem, size);
    }

    constexpr View<T> as_view()
    {
        return View<T>(this->get(), _size);
    }

private:
    const usize _size;
};

}  // namespace Memory
