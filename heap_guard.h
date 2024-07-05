#pragma once

#include <unistd.h>

#include <algorithm>
#include <cstdio>

#include "base.h"
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

    class Iter
    {
    public:
        using const_iterator = const u8*;
        const_iterator cbegin() { return _begin; }

        const_iterator cend() { return _end; }

    private:
        explicit Iter(const_iterator begin, const_iterator end) : _begin(begin), _end(end) {}
        const_iterator _begin;
        const_iterator _end;
        friend HeapGuard;
    };

    Iter iter() const { return Iter((u8*)_start, (u8*)sbrk(0)); }
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
    for (usize row = 0; row < heap_size; row += COLUMN_WIDTH)
    {
        const char* column_start = ((const char*)(_start)) + (row);

        // print addr
        printf("%zu\t| %p", row, column_start);
        printf(" | ");

        // print hex stuff
        for (usize column = 0; column < COLUMN_WIDTH; column++)
        {
            const char c = (char)(*(column_start + column));
            printf("0x%02x ", c);
        }
        printf(" | ");

        // print char stuff
        printf("%.*s", (int)COLUMN_WIDTH, column_start);
        printf("\n");
    }
}

#endif
