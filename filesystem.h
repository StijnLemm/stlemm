#pragma once

#include "owner.h"
#include "view_owner.h"

// Forward decl file. no stdlib needed.
struct __sFILE;
typedef __sFILE FILE;

namespace FileSystem
{
class FileHandle
{
public:
    ~FileHandle() noexcept;
    usize read_file_size() noexcept;

    FILE* _handle = nullptr;
    Memory::Owner<char> _path;
};

FileHandle open_file(const View<const char>& path);
Memory::ViewOwner<u8> read_file(FileHandle& handle);

}  // namespace FileSystem
