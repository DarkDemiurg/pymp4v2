#ifndef MP4_FILE_HANDLE_WRAPPER_H
#define MP4_FILE_HANDLE_WRAPPER_H

#include <cstdint>
#include <optional>

#include "mp4v2/mp4v2.h"
#include "pymp4v2/error.h"

namespace raw
{
    // RAII owner of MP4FileHandle. Copy is forbidden; get() throws after close.
    class MP4FileHandleWrapper
    {
    public:
        MP4FileHandleWrapper();
        explicit MP4FileHandleWrapper(MP4FileHandle h);

        MP4FileHandleWrapper(const MP4FileHandleWrapper &) = delete;
        MP4FileHandleWrapper &operator=(const MP4FileHandleWrapper &) = delete;

        MP4FileHandleWrapper(MP4FileHandleWrapper &&other) noexcept;
        MP4FileHandleWrapper &operator=(MP4FileHandleWrapper &&other) noexcept;

        ~MP4FileHandleWrapper();

        MP4FileHandle get() const;

        bool is_valid() const;

        // Close and invalidate. If flags is unset, uses close_flags().
        void close(std::optional<uint32_t> flags = std::nullopt);

        uint32_t close_flags() const;
        void set_close_flags(uint32_t flags);

    private:
        MP4FileHandle handle;
        uint32_t close_flags_ = 0;
    };

} // namespace raw

#endif // MP4_FILE_HANDLE_WRAPPER_H
