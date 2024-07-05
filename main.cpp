#include <unistd.h>

#include <cstdio>

// must be first
#include "heap_guard.h"
#include "view.h"

#define PRINT_DATA_BREAK View<void>(sbrk(0), 0).dump()

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

int main(int argc, char* argv[])
{
    HeapGuard::dump();

    char* new_data = (char*)naive_alloc(65);
    new_data[0] = 'h';
    new_data[1] = 'i';
    new_data[2] = '!';
    new_data[64] = 'E';

    HeapGuard::hex_dump();

    test_views();
}
