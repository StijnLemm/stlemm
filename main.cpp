#include <unistd.h>

#include <cstdio>
#include <cstdlib>

// must be first
#include "heap_guard.h"
#include "memory.h"
#include "owner.h"
#include "view.h"

void test_views()
{
    // hello world
    auto v1 = "Hello World!"_View;
    v1.dump();
}

void* naive_alloc(const usize size)
{
    void* prev = sbrk(0);  // this is what gets returned

    if (sbrk(size) == (void*)-1)
    {
        printf("Alloc failed, size requested: %zu\n", size);
        return nullptr;
    }

    return prev;
}

struct S
{
    i32 a;
    i32 b;
};

template <usize size>
void test_memory_copy()
{
    u8* new_data = (u8*)naive_alloc(size);
    u8 need_copy[size] = {0};
    for (usize i = 0; i < size; i++)
    {
        need_copy[i] = i;
    }
    Memory::copy(new_data, need_copy, size);
}

void test_memory_copy_typed()
{
    S* ptr = (S*)naive_alloc(sizeof(S) * 10);
    S array[10];
    for (i16 i = 0; i < 10; i++)
    {
        array[i] = {i, i};
    }
    Memory::copy(ptr, array, 10);
}

void fill_data(u8* data, const usize size, u8 c)
{
    for (usize i = 0; i < size; i++)
    {
        data[i] = c;
    }
}

void test_memory_alloc()
{
    // helper_memory_alloc(8);
    // helper_memory_alloc(16);
    // helper_memory_alloc(24);
    // for (int i = 8; i <= 64; i += 8)
    // {
    //     u8* data = Memory::alloc<u8>(i);
    //     fill_data(data, i);
    //     Memory::free(data);
    // }

    for (int i = 0; i <= 100; i++)
    {
        int needed_bytes = rand() % 100;
        u8* data = Memory::Heap::alloc<u8>(needed_bytes);
        fill_data(data, needed_bytes, '#');
        printf("\e[1;1H\e[2J");
        printf("Allocating: %d\n", needed_bytes);
        HeapGuard::hex_dump();
        usleep(1000 * 1000);
        if (needed_bytes % 8 == 0)
        {
            // leaks
            fill_data(data, needed_bytes, '!');
            continue;
        }

        fill_data(data, needed_bytes, '*');
        Memory::Heap::free(data);
    }

    // u8* data = Memory::Heap::alloc<u8>(16);
    // fill_data(data, 16, '#');
    // u8* data1 = Memory::Heap::alloc<u8>(16);
    // fill_data(data1, 16, '#');
    // fill_data(data1, 16);
    // Memory::Heap::free(data);
    // Memory::Heap::free(data1);

    // data = Memory::Heap::alloc<u8>(32 + 8);
    // fill_data(data, 32 + 8, '@');
}

void take(Memory::Owner<S> v) {}

int main(int argc, char* argv[])
{
    HeapGuard::dump();
    // test_views();
    // test_memory_copy<128>();
    // test_memory_alloc();
    auto p = Memory::Owner<S>::create(1, 2);
    take(Memory::move(p));
    // assert(p.get() == nullptr);

    HeapGuard::hex_dump();
}
