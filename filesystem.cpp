#include "filesystem.h"

#include <sys/stat.h>

#include <cstdio>

#include "base.h"
#include "memory.h"
#include "owner.h"

namespace FileSystem
{

FileHandle::~FileHandle() noexcept
{
    fclose(this->_handle);
}

usize FileHandle::read_file_size() noexcept
{
    struct stat st;
    stat(this->_path.get(), &st);
    return st.st_size;
}

FileHandle open_file(const View<const char>& path)
{
    auto p = path.c_str();
    FILE* handle = fopen(p.get(), "r");
    return FileHandle{handle, Memory::move(p)};
}

Memory::ViewOwner<u8> read_file(FileHandle& handle)
{
    const usize file_size = handle.read_file_size();
    auto buffer = Memory::ViewOwner<u8>::create(file_size);
    fread(buffer.get(), file_size, 1, handle._handle);
    return Memory::move(buffer);
}

}  // namespace FileSystem
