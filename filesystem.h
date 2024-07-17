#pragma once

#include "list.h"
#include "owner.h"
#include "result.h"

// Forward decl file. no stdlib needed.
struct __sFILE;
typedef __sFILE FILE;

namespace FileSystem
{

enum class FileOpError
{
    READ_EOF,
    READ_IO_FAIL,
    READ_GENERIC_FAIL,
};

class FileHandle
{
public:
    FileHandle(FILE* handle, Memory::Owner<char> path) noexcept;
    ~FileHandle() noexcept;

    FileHandle(FileHandle&& other) noexcept
    {
        this->_handle = other._handle;
        other._handle = nullptr;

        this->_path = Memory::move(other._path);
    }

    usize read_file_size() const noexcept;

    template <typename T>
    using ReadResult = Result<Memory::List<T>, FileOpError>;
    ReadResult<char> read_text() const noexcept;
    ReadResult<u8> read_binary() const noexcept;

private:
    FILE* _handle = nullptr;
    Memory::Owner<char> _path;
};

DefaultResult<FileHandle> get_file_handle(const View<const char>& path) noexcept;

}  // namespace FileSystem
