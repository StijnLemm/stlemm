#pragma once

#include <unistd.h>

#include "base.h"

#ifdef STLEMM_DEBUG
#include <cstdio>
#endif

class HeapGuard
{
public:
    static usize size()
    {
        void* data_brk = sbrk(0);
        return (usize)data_brk - (usize)_start;
    }

#ifdef STLEMM_DEBUG
    static void dump();
    static void hex_dump();
#endif

private:
    static inline const void* _start = sbrk(0);  // todo, static init order fiasco
};

#ifdef STLEMM_DEBUG
inline void HeapGuard::dump()
{
    void* data_brk = sbrk(0);
    printf("--- Heap guard info ---\n");
    printf("Heap start\t: %p\n", _start);
    printf("Heap end\t: %p\n", data_brk);
    printf("Heap size\t: %zu\n", size());
}

inline void HeapGuard::hex_dump()
{
    constexpr usize COLUMN_WIDTH = sizeof(usize);

    dump();
    printf("Hex dump\t:\n");

    const usize heap_size = size();
    for (usize byte_count = 0; byte_count < heap_size; byte_count += COLUMN_WIDTH)
    {
        const char* column_start = ((const char*)(_start)) + (byte_count);

        // print addr
        printf("%zu\t| %p", byte_count, column_start);
        printf(" | ");

        // print hex stuff
        for (usize column = 0; column < COLUMN_WIDTH; column++)
        {
            if (column + byte_count >= heap_size)
            {
                printf("     ");
                continue;
            }
            const u8 c = (u8)(*(column_start + column));
            printf("0x%02x ", c);
        }
        printf(" | ");

        // print char stuff
        for (usize column = 0; column < COLUMN_WIDTH; column++)
        {
            if (column + byte_count >= heap_size)
            {
                break;
            }
            const char c = (char)(*(column_start + column));
            if (c > 0x10)
            {
                printf("%c", c);
            }
            else
            {
                printf(" ");
            }
        }

        printf("\n");
    }
}

#endif
