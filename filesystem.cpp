#include "filesystem.h"

#include <sys/stat.h>

#include <cstdio>

#include "base.h"
#include "memory.h"
#include "owner.h"
#include "result.h"
#include "view_owner.h"

namespace FileSystem
{

FileHandle::FileHandle(FILE* handle, Memory::Owner<char> path) noexcept
    : _handle(handle), _path(Memory::move(path))
{
}

FileHandle::~FileHandle() noexcept
{
    if (this->_handle != nullptr)
    {
        fclose(this->_handle);
    }
}

usize FileHandle::read_file_size() const noexcept
{
    struct stat st;
    stat(this->_path.get(), &st);
    return st.st_size;
}

FileOpError retrieve_read_error(FILE* handle)
{
    if (ferror(handle))
    {
        return FileOpError::READ_IO_FAIL;
    }
    else if (feof(handle))
    {
        return FileOpError::READ_EOF;
    }

    return FileOpError::READ_GENERIC_FAIL;
}

template <typename T>
FileHandle::ReadResult<T> read(FILE* handle, const usize file_size)
{
    auto buffer = Memory::ViewOwner<T>::create(file_size);
    const usize bytes_read = fread(buffer.get(), 1, file_size, handle);

    // something went wrong if these do not match.
    if (bytes_read != file_size)
    {
        printf("File size: %zu, bytes read: %zu\n", file_size, bytes_read);
        return retrieve_read_error(handle);
    }

    return Memory::move(buffer);
}

FileHandle::ReadResult<char> FileHandle::read_text() const noexcept
{
    const usize file_size = this->read_file_size();
    return read<char>(this->_handle, file_size);
}

FileHandle::ReadResult<u8> FileHandle::read_binary() const noexcept
{
    const usize file_size = this->read_file_size();
    return read<u8>(this->_handle, file_size);
}

DefaultResult<FileHandle> get_file_handle(const View<const char>& path) noexcept
{
    auto p = path.c_str();
    FILE* handle = fopen(p.get(), "r");
    if (handle == nullptr)
    {
        return ResultError::FAIL;
    }

    return FileHandle(handle, Memory::move(p));
}

}  // namespace FileSystem
