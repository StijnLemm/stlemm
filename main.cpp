#include <unistd.h>

#include <cstdio>

#include "base.h"
#include "filesystem.h"
#include "heap_guard.h"
#include "memory.h"
#include "owner.h"

class LifeTime
{
public:
    // Default constructor
    LifeTime()
    {
        printf("[LifeTime] Default constructor called\n");
    }

    // Copy constructor
    LifeTime(const LifeTime&)
    {
        printf("[LifeTime] Copy constructor called\n");
    }

    // Move constructor
    LifeTime(LifeTime&&) noexcept
    {
        printf("[LifeTime] Move constructor called\n");
    }

    // Copy assignment operator
    LifeTime& operator=(const LifeTime&)
    {
        printf("[LifeTime] Copy assignment operator called\n");
        return *this;
    }

    // Move assignment operator
    LifeTime& operator=(LifeTime&&) noexcept
    {
        printf("[LifeTime] Move assignment operator called\n");
        return *this;
    }

    // Destructor
    ~LifeTime()
    {
        printf("[LifeTime] Destructor called\n");
    }
};

void test_views()
{
    // hello world
    auto v1 = "Hello World!"_View;
    v1.dump();
}

struct S
{
    i32 a;
    i32 b;
};

template <usize size>
void test_memory_copy()
{
    u8* new_data = Memory::Heap::alloc<u8>(size);
    u8 need_copy[size] = {0};
    for (usize i = 0; i < size; i++)
    {
        need_copy[i] = i;
    }
    Memory::copy(new_data, need_copy, size);
}

void test_memory_copy_typed()
{
    S* ptr = Memory::Heap::alloc<S>(10);
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

void clear_console()
{
    printf("\e[1;1H\e[2J");
}

void test_memory_alloc()
{
    // helper_memory_alloc(8);
    // helper_memory_alloc(16);
    // helper_memory_alloc(24);
    // for (int i = 1; i <= 100; i++)
    // {
    //     u8* data = Memory::Heap::alloc<u8>(i);
    //     fill_data(data, i, i + 0x30);
    //     Memory::Heap::free(data);
    //     printf("\e[1;1H\e[2J");
    //     HeapGuard::hex_dump();
    //     usleep(1000 * 1000);
    // }

    // for (int i = 0; i <= 100; i++)
    // {
    //     int needed_bytes = rand() % 100;
    //     u8* data = Memory::Heap::alloc<u8>(needed_bytes);
    //     fill_data(data, needed_bytes, '#');
    //     printf("\e[1;1H\e[2J");
    //     printf("Allocating: %d\n", needed_bytes);
    //     HeapGuard::hex_dump();
    //     usleep(1000 * 1000);
    //     if (needed_bytes % 8 == 0)
    //     {
    //         // leaks
    //         fill_data(data, needed_bytes, '!');
    //         continue;
    //     }
    //
    //     fill_data(data, needed_bytes, '*');
    //     Memory::Heap::free(data);
    // }

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

void test_list()
{
    // auto ptr = Memory::Heap::alloc<u8>(10);
    // auto ptr1 = Memory::Heap::alloc<u8>(10);
    //
    // auto list = Memory::List<int>::create(64);
    // for (auto& num : list.as_view())
    // {
    //     num = (usize)-1;
    // }
    // list.reserve(120);

    auto list = Memory::List<u8>::create();
    for (int i = 0; i < 256; i++)
    {
        list.add(i);
    }
}

void test_realloc()
{
    auto ptr = Memory::Heap::alloc<u8>(10);
    auto ptr1 = Memory::Heap::alloc<u8>(10);
    auto ptr2 = Memory::Heap::alloc<u8>(10);
    auto ptr3 = Memory::Heap::alloc<u8>(10);

    // add to tail
    auto r_ptr = Memory::Heap::realloc(ptr3, 128);
    expect(r_ptr == ptr3, "must stay the same ptr");

    // auto r_ptr = Memory::Heap::realloc(ptr, 65);
    // expect(r_ptr == ptr, "must stay the same ptr");
    //
    // Memory::Heap::free(ptr1);
    //
    // r_ptr = Memory::Heap::realloc(ptr, 128);
    // expect(r_ptr == ptr, "must stay the same ptr");
    //
    // Memory::Heap::free(ptr2);
    //
    // r_ptr = Memory::Heap::realloc(ptr, 140);
    // expect(r_ptr == ptr, "must stay the same ptr");
}

void take(Memory::Owner<LifeTime> v)
{
    printf("Inside function to use v!\n");
}

enum class FilesystemError
{
    FAILED,
};

using FileSystemResult = Result<Memory::List<char>, FilesystemError>;

FileSystemResult get(bool fail)
{
    if (fail)
    {
        return FilesystemError::FAILED;
    }

    return Memory::List<char>::create(10);
}

void on_file_read(const FileSystem::FileHandle& handle)
{
    handle.read_text().visit([](auto text) { text.as_view().dump(); },
                             [](auto error) { printf("Error: %d\n", static_cast<int>(error)); });
}

void on_file_read2(const FileSystem::FileHandle& handle)
{
    handle.read_binary().visit([](auto text) { text.as_view().dump(); },
                               [](auto error) { printf("Error: %d\n", static_cast<int>(error)); });
}

int main(int argc, char* argv[])
{
    // FileSystem::get_file_handle("/Users/slemm/Documents/dev/notes/example.md"_View)
    //     .visit([](const auto& handle) { on_file_read2(handle); },
    //            [](auto error) { printf("Error reading file, %d\n", ascast(error, int)); });

    test_list();
    // test_realloc();

    HeapGuard::hex_dump();
}
