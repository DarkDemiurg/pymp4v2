#ifndef PYMP4V2_MP4_TAGS_WRAPPER_H
#define PYMP4V2_MP4_TAGS_WRAPPER_H

#include "mp4v2/mp4v2.h"

namespace raw
{
    // RAII owner of MP4TagsAlloc / MP4TagsFree. Move-only; get() throws after free.
    class MP4TagsWrapper
    {
    public:
        MP4TagsWrapper();
        ~MP4TagsWrapper();

        MP4TagsWrapper(const MP4TagsWrapper &) = delete;
        MP4TagsWrapper &operator=(const MP4TagsWrapper &) = delete;
        MP4TagsWrapper(MP4TagsWrapper &&other) noexcept;
        MP4TagsWrapper &operator=(MP4TagsWrapper &&other) noexcept;

        const MP4Tags *get() const;
        bool is_valid() const;
        void free();

    private:
        const MP4Tags *tags;
    };
} // namespace raw

#endif // PYMP4V2_MP4_TAGS_WRAPPER_H
