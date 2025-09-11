#include "pymp4v2/mp4_file_handle_wrapper.h"

namespace raw
{
    MP4FileHandleWrapper::MP4FileHandleWrapper() : handle(nullptr) {}

    MP4FileHandleWrapper::MP4FileHandleWrapper(MP4FileHandle h) : handle(h) {}

    // Реализация перемещения
    MP4FileHandleWrapper::MP4FileHandleWrapper(MP4FileHandleWrapper &&other) noexcept
        : handle(other.handle)
    {
        other.handle = nullptr;
    }

    MP4FileHandleWrapper &MP4FileHandleWrapper::operator=(MP4FileHandleWrapper &&other) noexcept
    {
        if (this != &other)
        {
            close(0); // Закрываем текущий файл, если открыт
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }

    MP4FileHandleWrapper::~MP4FileHandleWrapper()
    {
        if (handle != nullptr)
        {
            MP4Close(handle, 0);
        }
    }

    MP4FileHandle MP4FileHandleWrapper::get() const
    {
        if (handle == nullptr)
        {
            throw std::runtime_error("MP4FileHandle is no longer valid (file has been closed)");
        }
        return handle;
    }

    bool MP4FileHandleWrapper::is_valid() const
    {
        return handle != nullptr;
    }

    void MP4FileHandleWrapper::close(uint32_t flags)
    {
        if (handle != nullptr)
        {
            MP4Close(handle, flags);
            handle = nullptr;
        }
    }
} // namespace raw
