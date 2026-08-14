#include "pymp4v2/mp4_file_handle_wrapper.h"

namespace raw
{
    MP4FileHandleWrapper::MP4FileHandleWrapper() : handle(nullptr) {}

    MP4FileHandleWrapper::MP4FileHandleWrapper(MP4FileHandle h) : handle(h) {}

    MP4FileHandleWrapper::MP4FileHandleWrapper(MP4FileHandleWrapper &&other) noexcept
        : handle(other.handle), close_flags_(other.close_flags_)
    {
        other.handle = nullptr;
    }

    MP4FileHandleWrapper &MP4FileHandleWrapper::operator=(MP4FileHandleWrapper &&other) noexcept
    {
        if (this != &other)
        {
            close();
            handle = other.handle;
            close_flags_ = other.close_flags_;
            other.handle = nullptr;
        }
        return *this;
    }

    MP4FileHandleWrapper::~MP4FileHandleWrapper()
    {
        if (handle != nullptr)
        {
            MP4Close(handle, close_flags_);
        }
    }

    MP4FileHandle MP4FileHandleWrapper::get() const
    {
        if (handle == nullptr)
        {
            throw MP4Error("MP4FileHandle is no longer valid (file has been closed)");
        }
        return handle;
    }

    bool MP4FileHandleWrapper::is_valid() const
    {
        return handle != nullptr;
    }

    void MP4FileHandleWrapper::close(std::optional<uint32_t> flags)
    {
        if (handle != nullptr)
        {
            MP4Close(handle, flags.value_or(close_flags_));
            handle = nullptr;
        }
    }

    uint32_t MP4FileHandleWrapper::close_flags() const
    {
        return close_flags_;
    }

    void MP4FileHandleWrapper::set_close_flags(uint32_t flags)
    {
        close_flags_ = flags;
    }
} // namespace raw
