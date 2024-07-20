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
    List(T* ptr, const usize cap) : Owner<T>(ptr), _cap(cap) {}

    // NOTE: some default init cap
    static List<T> create(const usize initial_size = 32u)
    {
        auto mem = Memory::Heap::alloc<T>(initial_size);
        return List<T>(mem, initial_size);
    }

    void reserve(const usize size)
    {
        this->_ptr = Memory::Heap::realloc(this->_ptr, size);

        // NOTE: this realloc could fail. Adjusting cap is unwise.
        if (this->_ptr == nullptr) return;

        this->_cap = size;
    }

    T& operator[](const usize index)
    {
        return this->get()[index];
    }

    const T& operator[](const usize index) const
    {
        return this->get()[index];
    }

    constexpr void add(T item)
    {
        if (_size < _cap)
        {
            (*this)[this->_size] = item;
            this->_size++;
            return;
        }

        // TODO: create a constant which represets this value?
        this->reserve(_cap + 32u);
        this->add(item);
    }

    constexpr View<T> as_view()
    {
        return View<T>(this->get(), _size);
    }

private:
    usize _cap;
    usize _size = 0;
};

}  // namespace Memory
