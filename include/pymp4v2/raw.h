#ifndef PYMP4V2_RAW_H
#define PYMP4V2_RAW_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "mp4v2/mp4v2.h"
#include "pymp4v2/error.h"
#include "pymp4v2/mp4_file_handle_wrapper.h"
#include "pymp4v2/mp4_tags_wrapper.h"

namespace py = pybind11;

namespace raw
{
    // Copy an MP4v2-allocated C string into std::string and MP4Free the original.
    inline std::optional<std::string> take_allocated_string(const char *info)
    {
        if (info == nullptr)
        {
            return std::nullopt;
        }
        std::string result(info);
        MP4Free(const_cast<char *>(info));
        return result;
    }

    // Copy an MP4v2-allocated byte buffer into std::string and MP4Free the original.
    inline std::string take_allocated_bytes(uint8_t *bytes, uint32_t size)
    {
        std::string result;
        if (bytes != nullptr && size > 0)
        {
            result.assign(reinterpret_cast<char *>(bytes), size);
        }
        if (bytes != nullptr)
        {
            MP4Free(bytes);
        }
        return result;
    }

    template <typename T>
    void MP4TagsSetScalar_wrapper(MP4TagsWrapper &tags, bool (*setter)(const MP4Tags *, const T *),
                                  const std::optional<T> &value, const char *what)
    {
        const T *p = value ? &*value : nullptr;
        if (!setter(tags.get(), p))
        {
            throw MP4Error(std::string(what) + " failed");
        }
    }

    struct MP4SampleData
    {
        std::string data;
        MP4Timestamp startTime;
        MP4Duration duration;
        MP4Duration renderingOffset;
        bool isSyncSample;
    };

    // fileName
    MP4FileHandleWrapper MP4Read_wrapper(const char *fileName);
    MP4FileHandleWrapper MP4Create_wrapper(const char *fileName, uint32_t flags = 0);
    MP4FileHandleWrapper MP4CreateEx_wrapper(const char *fileName, uint32_t flags,
                                             int add_ftyp, int add_iods,
                                             const std::optional<std::string> &majorBrand,
                                             uint32_t minorVersion,
                                             const std::vector<std::string> &compatibleBrands);
    MP4FileHandleWrapper MP4Modify_wrapper(const char *fileName, uint32_t flags = 0);
    std::optional<std::string> MP4FileInfo_wrapper(const char *fileName, MP4TrackId trackId = MP4_INVALID_TRACK_ID);
    bool MP4Optimize_wrapper(const char *fileName, const char *newFileName = NULL);

    // file handle
    void MP4Close_wrapper(MP4FileHandleWrapper &hFile, uint32_t flags = 0);
    const char *MP4GetFilename_wrapper(MP4FileHandleWrapper &hFile);
    bool MP4Dump_wrapper(MP4FileHandleWrapper &hFile, bool dumpImplicits = false);
    std::optional<std::string> MP4Info_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId);

    // tracks
    uint32_t MP4GetNumberOfTracks_wrapper(MP4FileHandleWrapper &hFile, const std::optional<std::string> &type,
                                          uint8_t subType);
    MP4TrackId MP4FindTrackId_wrapper(MP4FileHandleWrapper &hFile, uint16_t index,
                                      const std::optional<std::string> &type, uint8_t subType);
    std::string MP4GetTrackType_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId);
    void MP4DeleteTrack_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId);
    MP4TrackId MP4AddAudioTrack_wrapper(MP4FileHandleWrapper &hFile, uint32_t timeScale, MP4Duration sampleDuration,
                                        uint8_t audioType);
    MP4TrackId MP4AddVideoTrack_wrapper(MP4FileHandleWrapper &hFile, uint32_t timeScale, MP4Duration sampleDuration,
                                        uint16_t width, uint16_t height, uint8_t videoType);

    // file / track properties
    bool MP4HaveAtom_wrapper(MP4FileHandleWrapper &hFile, const char *atomName);
    MP4Duration MP4GetDuration_wrapper(MP4FileHandleWrapper &hFile);
    uint32_t MP4GetTimeScale_wrapper(MP4FileHandleWrapper &hFile);
    void MP4SetTimeScale_wrapper(MP4FileHandleWrapper &hFile, uint32_t value);
    MP4Duration MP4GetTrackDuration_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId);
    uint32_t MP4GetTrackTimeScale_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId);
    void MP4SetTrackTimeScale_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, uint32_t value);
    uint16_t MP4GetTrackVideoWidth_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId);
    uint16_t MP4GetTrackVideoHeight_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId);
    std::string MP4GetTrackLanguage_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId);
    void MP4SetTrackLanguage_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *code);
    std::string MP4GetTrackName_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId);
    void MP4SetTrackName_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *name);
    void MP4ChangeMovieTimeScale_wrapper(MP4FileHandleWrapper &hFile, uint32_t value);
    uint64_t MP4GetIntegerProperty_wrapper(MP4FileHandleWrapper &hFile, const char *propName);
    float MP4GetFloatProperty_wrapper(MP4FileHandleWrapper &hFile, const char *propName);
    std::string MP4GetStringProperty_wrapper(MP4FileHandleWrapper &hFile, const char *propName);
    std::string MP4GetBytesProperty_wrapper(MP4FileHandleWrapper &hFile, const char *propName);
    void MP4SetIntegerProperty_wrapper(MP4FileHandleWrapper &hFile, const char *propName, int64_t value);
    void MP4SetFloatProperty_wrapper(MP4FileHandleWrapper &hFile, const char *propName, float value);
    void MP4SetStringProperty_wrapper(MP4FileHandleWrapper &hFile, const char *propName, const char *value);
    void MP4SetBytesProperty_wrapper(MP4FileHandleWrapper &hFile, const char *propName, const std::string &data);
    bool MP4HaveTrackAtom_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *atomName);
    std::string MP4GetTrackMediaDataName_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId);
    MP4Duration MP4GetTrackFixedSampleDuration_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId);
    uint32_t MP4GetTrackBitRate_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId);
    double MP4GetTrackVideoFrameRate_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId);
    int MP4GetTrackAudioChannels_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId);
    std::string MP4GetTrackESConfiguration_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId);
    void MP4SetTrackESConfiguration_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const std::string &data);
    uint64_t MP4GetTrackIntegerProperty_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *propName);
    float MP4GetTrackFloatProperty_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *propName);
    std::string MP4GetTrackStringProperty_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *propName);
    std::string MP4GetTrackBytesProperty_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *propName);
    void MP4SetTrackIntegerProperty_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *propName,
                                            int64_t value);
    void MP4SetTrackFloatProperty_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *propName,
                                          float value);
    void MP4SetTrackStringProperty_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *propName,
                                           const char *value);
    void MP4SetTrackBytesProperty_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *propName,
                                          const std::string &data);

    // samples
    MP4SampleData MP4ReadSample_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, MP4SampleId sampleId);
    MP4SampleData MP4ReadSampleFromTime_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, MP4Timestamp when);
    void MP4WriteSample_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const std::string &data,
                                MP4Duration duration, MP4Duration renderingOffset, bool isSyncSample);
    void MP4CopySample_wrapper(MP4FileHandleWrapper &srcFile, MP4TrackId srcTrackId, MP4SampleId srcSampleId,
                               MP4FileHandleWrapper *dstFile, MP4TrackId dstTrackId, MP4Duration dstSampleDuration);
    uint32_t MP4GetSampleSize_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, MP4SampleId sampleId);
    MP4SampleId MP4GetSampleIdFromTime_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, MP4Timestamp when,
                                               bool wantSyncSample);
    MP4Timestamp MP4GetSampleTime_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, MP4SampleId sampleId);
    MP4Duration MP4GetSampleDuration_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, MP4SampleId sampleId);
    bool MP4GetSampleSync_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, MP4SampleId sampleId);
    MP4SampleId MP4GetTrackNumberOfSamples_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId);
    uint32_t MP4GetTrackMaxSampleSize_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId);

    // iTMF tags
    void MP4TagsFetch_wrapper(MP4TagsWrapper &tags, MP4FileHandleWrapper &hFile);
    void MP4TagsStore_wrapper(MP4TagsWrapper &tags, MP4FileHandleWrapper &hFile);
    bool MP4TagsHasMetadata_wrapper(MP4TagsWrapper &tags);
    void MP4TagsSetString_wrapper(MP4TagsWrapper &tags, bool (*setter)(const MP4Tags *, const char *),
                                  const std::optional<std::string> &value, const char *what);
    void MP4TagsAddArtwork_wrapper(MP4TagsWrapper &tags, const std::string &data, MP4TagArtworkType type);
    void MP4TagsSetArtwork_wrapper(MP4TagsWrapper &tags, uint32_t index, const std::string &data,
                                   MP4TagArtworkType type);
    void MP4TagsRemoveArtwork_wrapper(MP4TagsWrapper &tags, uint32_t index);

    // chapters
    struct MP4ChapterData
    {
        MP4Duration duration = 0;
        std::string title;

        MP4ChapterData() = default;
        MP4ChapterData(MP4Duration duration, std::string title) : duration(duration), title(std::move(title)) {}
    };

    struct MP4ChapterListResult
    {
        MP4ChapterType type = MP4ChapterTypeNone;
        std::vector<MP4ChapterData> chapters;
    };

    MP4TrackId MP4AddChapterTextTrack_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId refTrackId, uint32_t timescale);
    void MP4AddChapter_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId chapterTrackId, MP4Duration chapterDuration,
                               const std::optional<std::string> &chapterTitle);
    void MP4AddNeroChapter_wrapper(MP4FileHandleWrapper &hFile, MP4Timestamp chapterStart,
                                   const std::optional<std::string> &chapterTitle);
    MP4ChapterType MP4ConvertChapters_wrapper(MP4FileHandleWrapper &hFile, MP4ChapterType toChapterType);
    MP4ChapterType MP4DeleteChapters_wrapper(MP4FileHandleWrapper &hFile, MP4ChapterType chapterType,
                                             MP4TrackId chapterTrackId);
    MP4ChapterListResult MP4GetChapters_wrapper(MP4FileHandleWrapper &hFile, MP4ChapterType chapterType);
    MP4ChapterType MP4SetChapters_wrapper(MP4FileHandleWrapper &hFile, const std::vector<MP4ChapterData> &chapters,
                                          MP4ChapterType chapterType);

    void bind_extended(py::module_ &m_raw);
} // namespace raw

#endif // PYMP4V2_RAW_H
