#include <unistd.h>

#include <cstdio>

// must be first
#include "heap_guard.h"
#include "memory.h"
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
    i16 a;
    i16 b;
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

void test_memory_alloc()
{
    u8* data = Memory::alloc<u8>(8);
}

int main(int argc, char* argv[])
{
    HeapGuard::dump();
    // test_views();
    // test_memory_copy<128>();
    test_memory_alloc();
    HeapGuard::hex_dump();
}
