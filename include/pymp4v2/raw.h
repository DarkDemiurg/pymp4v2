#ifndef PYMP4V2_RAW_H
#define PYMP4V2_RAW_H

#include <string>
#include <cstdint>
#include <optional>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "pymp4v2/mp4_file_handle_wrapper.h"

namespace py = pybind11;

namespace raw
{
    // with fileName
    MP4FileHandleWrapper MP4Read_wrapper(const char *fileName);
    MP4FileHandleWrapper MP4Create_wrapper(const char *fileName, uint32_t flags = 0);
    MP4FileHandleWrapper MP4Modify_wrapper(const char *fileName, uint32_t flags = 0);
    std::optional<std::string> MP4FileInfo_wrapper(const char *fileName, MP4TrackId trackId = MP4_INVALID_TRACK_ID);
    bool MP4Optimize_wrapper(const char *fileName, const char *newFileName = NULL);
    // with file handle
    void MP4Close_wrapper(MP4FileHandleWrapper &hFile, uint32_t flags = 0);
    const char *MP4GetFilename_wrapper(MP4FileHandleWrapper &hFile);
    bool MP4Dump_wrapper(MP4FileHandleWrapper &hFile, bool dumpImplicits = false);
    std::optional<std::string> MP4Info_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId);
} // namespace raw

#endif // PYMP4V2_RAW_H
