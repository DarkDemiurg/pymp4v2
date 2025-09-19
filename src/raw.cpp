#include <stdexcept>

#include "mp4v2/mp4v2.h"
#include "pymp4v2/raw.h"

namespace py = pybind11;

namespace raw
{
    MP4FileHandleWrapper MP4Read_wrapper(const char *fileName)
    {
        MP4FileHandle hFile = MP4Read(fileName);
        if (hFile == nullptr)
        {
            throw std::runtime_error("Failed to open MP4 file: " + std::string(fileName));
        }

        return MP4FileHandleWrapper(hFile);
    }

    MP4FileHandleWrapper MP4Create_wrapper(const char *fileName, uint32_t flags)
    {
        MP4FileHandle hFile = MP4Create(fileName, flags);
        if (hFile == nullptr)
        {
            throw std::runtime_error("Failed to create MP4 file: " + std::string(fileName));
        }

        return MP4FileHandleWrapper(hFile);
    }

    MP4FileHandleWrapper MP4Modify_wrapper(const char *fileName, uint32_t flags)
    {
        MP4FileHandle hFile = MP4Modify(fileName, flags);
        if (hFile == nullptr)
        {
            throw std::runtime_error("Failed to modify MP4 file: " + std::string(fileName));
        }

        return MP4FileHandleWrapper(hFile);
    }

    std::optional<std::string> MP4FileInfo_wrapper(const char *fileName, MP4TrackId trackId)
    {
        const char *info = MP4FileInfo(fileName, trackId);
        if (info == nullptr)
        {
            return std::nullopt;
        }
        return std::string(info);
    }

    bool MP4Optimize_wrapper(const char *fileName, const char *newFileName)
    {
        return MP4Optimize(fileName, newFileName);
    }

    void MP4Close_wrapper(MP4FileHandleWrapper &hFile, uint32_t flags)
    {
        hFile.close(flags);
    }

    const char *MP4GetFilename_wrapper(MP4FileHandleWrapper &hFile)
    {
        return MP4GetFilename(hFile.get());
    }

    bool MP4Dump_wrapper(MP4FileHandleWrapper &hFile, bool dumpImplicits)
    {
        return MP4Dump(hFile.get(), dumpImplicits);
    }

    std::optional<std::string> MP4Info_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId)
    {
        const char *info = MP4Info(hFile.get(), trackId);
        if (info == nullptr)
        {
            return std::nullopt;
        }
        return std::string(info);
    }
} // namespace raw
