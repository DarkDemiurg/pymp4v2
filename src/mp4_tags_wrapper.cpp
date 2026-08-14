#include "pymp4v2/mp4_tags_wrapper.h"
#include "pymp4v2/error.h"

namespace raw
{
    MP4TagsWrapper::MP4TagsWrapper() : tags(MP4TagsAlloc())
    {
        if (tags == nullptr)
        {
            throw MP4Error("MP4TagsAlloc failed");
        }
    }

    MP4TagsWrapper::~MP4TagsWrapper()
    {
        free();
    }

    MP4TagsWrapper::MP4TagsWrapper(MP4TagsWrapper &&other) noexcept : tags(other.tags)
    {
        other.tags = nullptr;
    }

    MP4TagsWrapper &MP4TagsWrapper::operator=(MP4TagsWrapper &&other) noexcept
    {
        if (this != &other)
        {
            free();
            tags = other.tags;
            other.tags = nullptr;
        }
        return *this;
    }

    const MP4Tags *MP4TagsWrapper::get() const
    {
        if (tags == nullptr)
        {
            throw MP4Error("MP4Tags is no longer valid (already freed)");
        }
        return tags;
    }

    bool MP4TagsWrapper::is_valid() const
    {
        return tags != nullptr;
    }

    void MP4TagsWrapper::free()
    {
        if (tags != nullptr)
        {
            MP4TagsFree(tags);
            tags = nullptr;
        }
    }
} // namespace raw
