#include "mp4v2/mp4v2.h"
#include "pymp4v2/error.h"
#include "pymp4v2/raw.h"

#include <cstring>

namespace raw
{
    namespace
    {
        void require(bool ok, const char *what)
        {
            if (!ok)
            {
                throw MP4Error(std::string(what) + " failed");
            }
        }

        MP4TrackId require_track(MP4TrackId id, const char *what)
        {
            if (id == MP4_INVALID_TRACK_ID)
            {
                throw MP4Error(std::string(what) + " failed");
            }
            return id;
        }

        const char *optional_cstr(const std::optional<std::string> &value)
        {
            return value ? value->c_str() : nullptr;
        }

        MP4SampleId require_sample(MP4SampleId id, const char *what)
        {
            if (id == MP4_INVALID_SAMPLE_ID)
            {
                throw MP4Error(std::string(what) + " failed");
            }
            return id;
        }

        MP4SampleData finish_read_sample(bool ok, const char *what, uint8_t *bytes, uint32_t numBytes,
                                         MP4Timestamp startTime, MP4Duration duration, MP4Duration renderingOffset,
                                         bool isSyncSample)
        {
            if (!ok)
            {
                if (bytes != nullptr)
                {
                    MP4Free(bytes);
                }
                throw MP4Error(std::string(what) + " failed");
            }
            MP4SampleData sample;
            if (bytes != nullptr && numBytes > 0)
            {
                sample.data.assign(reinterpret_cast<char *>(bytes), numBytes);
            }
            if (bytes != nullptr)
            {
                MP4Free(bytes);
            }
            sample.startTime = startTime;
            sample.duration = duration;
            sample.renderingOffset = renderingOffset;
            sample.isSyncSample = isSyncSample;
            return sample;
        }
    } // namespace

    MP4FileHandleWrapper MP4Read_wrapper(const char *fileName)
    {
        MP4FileHandle hFile = MP4Read(fileName);
        if (hFile == nullptr)
        {
            throw MP4Error("Failed to open MP4 file: " + std::string(fileName));
        }

        return MP4FileHandleWrapper(hFile);
    }

    MP4FileHandleWrapper MP4Create_wrapper(const char *fileName, uint32_t flags)
    {
        MP4FileHandle hFile = MP4Create(fileName, flags);
        if (hFile == nullptr)
        {
            throw MP4Error("Failed to create MP4 file: " + std::string(fileName));
        }

        return MP4FileHandleWrapper(hFile);
    }

    MP4FileHandleWrapper MP4CreateEx_wrapper(const char *fileName, uint32_t flags, int add_ftyp,
                                             int add_iods, const std::optional<std::string> &majorBrand,
                                             uint32_t minorVersion,
                                             const std::vector<std::string> &compatibleBrands)
    {
        char *major = majorBrand ? const_cast<char *>(majorBrand->c_str()) : nullptr;
        std::vector<char *> brand_ptrs;
        brand_ptrs.reserve(compatibleBrands.size());
        for (const auto &brand : compatibleBrands)
        {
            brand_ptrs.push_back(const_cast<char *>(brand.c_str()));
        }
        char **brands = brand_ptrs.empty() ? nullptr : brand_ptrs.data();
        MP4FileHandle hFile = MP4CreateEx(fileName, flags, add_ftyp, add_iods, major, minorVersion,
                                          brands, static_cast<uint32_t>(compatibleBrands.size()));
        if (hFile == nullptr)
        {
            throw MP4Error("Failed to create MP4 file: " + std::string(fileName));
        }

        return MP4FileHandleWrapper(hFile);
    }

    MP4FileHandleWrapper MP4Modify_wrapper(const char *fileName, uint32_t flags)
    {
        MP4FileHandle hFile = MP4Modify(fileName, flags);
        if (hFile == nullptr)
        {
            throw MP4Error("Failed to modify MP4 file: " + std::string(fileName));
        }

        return MP4FileHandleWrapper(hFile);
    }

    std::optional<std::string> MP4FileInfo_wrapper(const char *fileName, MP4TrackId trackId)
    {
        return take_allocated_string(MP4FileInfo(fileName, trackId));
    }

    bool MP4Optimize_wrapper(const char *fileName, const char *newFileName)
    {
        require(MP4Optimize(fileName, newFileName), "MP4Optimize");
        return true;
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
        require(MP4Dump(hFile.get(), dumpImplicits), "MP4Dump");
        return true;
    }

    std::optional<std::string> MP4Info_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId)
    {
        return take_allocated_string(MP4Info(hFile.get(), trackId));
    }

    uint32_t MP4GetNumberOfTracks_wrapper(MP4FileHandleWrapper &hFile, const std::optional<std::string> &type,
                                          uint8_t subType)
    {
        MP4FileHandle h = hFile.get();
        const char *type_c = optional_cstr(type);
        py::gil_scoped_release release;
        return MP4GetNumberOfTracks(h, type_c, subType);
    }

    MP4TrackId MP4FindTrackId_wrapper(MP4FileHandleWrapper &hFile, uint16_t index,
                                      const std::optional<std::string> &type, uint8_t subType)
    {
        MP4FileHandle h = hFile.get();
        const char *type_c = optional_cstr(type);
        MP4TrackId id;
        {
            py::gil_scoped_release release;
            id = MP4FindTrackId(h, index, type_c, subType);
        }
        return require_track(id, "MP4FindTrackId");
    }

    std::string MP4GetTrackType_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId)
    {
        MP4FileHandle h = hFile.get();
        const char *type;
        {
            py::gil_scoped_release release;
            type = MP4GetTrackType(h, trackId);
        }
        if (type == nullptr)
        {
            throw MP4Error("MP4GetTrackType failed");
        }
        return std::string(type);
    }

    void MP4DeleteTrack_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId)
    {
        MP4FileHandle h = hFile.get();
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4DeleteTrack(h, trackId);
        }
        require(ok, "MP4DeleteTrack");
    }

    MP4TrackId MP4AddAudioTrack_wrapper(MP4FileHandleWrapper &hFile, uint32_t timeScale, MP4Duration sampleDuration,
                                        uint8_t audioType)
    {
        MP4FileHandle h = hFile.get();
        MP4TrackId id;
        {
            py::gil_scoped_release release;
            id = MP4AddAudioTrack(h, timeScale, sampleDuration, audioType);
        }
        return require_track(id, "MP4AddAudioTrack");
    }

    MP4TrackId MP4AddVideoTrack_wrapper(MP4FileHandleWrapper &hFile, uint32_t timeScale, MP4Duration sampleDuration,
                                        uint16_t width, uint16_t height, uint8_t videoType)
    {
        MP4FileHandle h = hFile.get();
        MP4TrackId id;
        {
            py::gil_scoped_release release;
            id = MP4AddVideoTrack(h, timeScale, sampleDuration, width, height, videoType);
        }
        return require_track(id, "MP4AddVideoTrack");
    }

    bool MP4HaveAtom_wrapper(MP4FileHandleWrapper &hFile, const char *atomName)
    {
        MP4FileHandle h = hFile.get();
        py::gil_scoped_release release;
        return MP4HaveAtom(h, atomName);
    }

    MP4Duration MP4GetDuration_wrapper(MP4FileHandleWrapper &hFile)
    {
        MP4FileHandle h = hFile.get();
        py::gil_scoped_release release;
        return MP4GetDuration(h);
    }

    uint32_t MP4GetTimeScale_wrapper(MP4FileHandleWrapper &hFile)
    {
        MP4FileHandle h = hFile.get();
        py::gil_scoped_release release;
        return MP4GetTimeScale(h);
    }

    void MP4SetTimeScale_wrapper(MP4FileHandleWrapper &hFile, uint32_t value)
    {
        MP4FileHandle h = hFile.get();
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4SetTimeScale(h, value);
        }
        require(ok, "MP4SetTimeScale");
    }

    MP4Duration MP4GetTrackDuration_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId)
    {
        MP4FileHandle h = hFile.get();
        py::gil_scoped_release release;
        return MP4GetTrackDuration(h, trackId);
    }

    uint32_t MP4GetTrackTimeScale_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId)
    {
        MP4FileHandle h = hFile.get();
        py::gil_scoped_release release;
        return MP4GetTrackTimeScale(h, trackId);
    }

    void MP4SetTrackTimeScale_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, uint32_t value)
    {
        MP4FileHandle h = hFile.get();
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4SetTrackTimeScale(h, trackId, value);
        }
        require(ok, "MP4SetTrackTimeScale");
    }

    uint16_t MP4GetTrackVideoWidth_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId)
    {
        MP4FileHandle h = hFile.get();
        py::gil_scoped_release release;
        return MP4GetTrackVideoWidth(h, trackId);
    }

    uint16_t MP4GetTrackVideoHeight_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId)
    {
        MP4FileHandle h = hFile.get();
        py::gil_scoped_release release;
        return MP4GetTrackVideoHeight(h, trackId);
    }

    std::string MP4GetTrackLanguage_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId)
    {
        MP4FileHandle h = hFile.get();
        char code[4] = {};
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4GetTrackLanguage(h, trackId, code);
        }
        require(ok, "MP4GetTrackLanguage");
        return std::string(code);
    }

    void MP4SetTrackLanguage_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *code)
    {
        MP4FileHandle h = hFile.get();
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4SetTrackLanguage(h, trackId, code);
        }
        require(ok, "MP4SetTrackLanguage");
    }

    std::string MP4GetTrackName_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId)
    {
        MP4FileHandle h = hFile.get();
        char *name = nullptr;
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4GetTrackName(h, trackId, &name);
        }
        if (!ok || name == nullptr)
        {
            if (name != nullptr)
            {
                MP4Free(name);
            }
            throw MP4Error("MP4GetTrackName failed");
        }
        std::string result(name);
        MP4Free(name);
        return result;
    }

    void MP4SetTrackName_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *name)
    {
        MP4FileHandle h = hFile.get();
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4SetTrackName(h, trackId, name);
        }
        require(ok, "MP4SetTrackName");
    }

    void MP4ChangeMovieTimeScale_wrapper(MP4FileHandleWrapper &hFile, uint32_t value)
    {
        MP4FileHandle h = hFile.get();
        py::gil_scoped_release release;
        MP4ChangeMovieTimeScale(h, value);
    }

    uint64_t MP4GetIntegerProperty_wrapper(MP4FileHandleWrapper &hFile, const char *propName)
    {
        MP4FileHandle h = hFile.get();
        uint64_t value = 0;
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4GetIntegerProperty(h, propName, &value);
        }
        require(ok, "MP4GetIntegerProperty");
        return value;
    }

    float MP4GetFloatProperty_wrapper(MP4FileHandleWrapper &hFile, const char *propName)
    {
        MP4FileHandle h = hFile.get();
        float value = 0;
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4GetFloatProperty(h, propName, &value);
        }
        require(ok, "MP4GetFloatProperty");
        return value;
    }

    std::string MP4GetStringProperty_wrapper(MP4FileHandleWrapper &hFile, const char *propName)
    {
        MP4FileHandle h = hFile.get();
        const char *value = nullptr;
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4GetStringProperty(h, propName, &value);
        }
        require(ok, "MP4GetStringProperty");
        if (value == nullptr)
        {
            throw MP4Error("MP4GetStringProperty failed");
        }
        return std::string(value);
    }

    std::string MP4GetBytesProperty_wrapper(MP4FileHandleWrapper &hFile, const char *propName)
    {
        MP4FileHandle h = hFile.get();
        uint8_t *bytes = nullptr;
        uint32_t size = 0;
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4GetBytesProperty(h, propName, &bytes, &size);
        }
        if (!ok)
        {
            if (bytes != nullptr)
            {
                MP4Free(bytes);
            }
            throw MP4Error("MP4GetBytesProperty failed");
        }
        return take_allocated_bytes(bytes, size);
    }

    void MP4SetIntegerProperty_wrapper(MP4FileHandleWrapper &hFile, const char *propName, int64_t value)
    {
        MP4FileHandle h = hFile.get();
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4SetIntegerProperty(h, propName, value);
        }
        require(ok, "MP4SetIntegerProperty");
    }

    void MP4SetFloatProperty_wrapper(MP4FileHandleWrapper &hFile, const char *propName, float value)
    {
        MP4FileHandle h = hFile.get();
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4SetFloatProperty(h, propName, value);
        }
        require(ok, "MP4SetFloatProperty");
    }

    void MP4SetStringProperty_wrapper(MP4FileHandleWrapper &hFile, const char *propName, const char *value)
    {
        MP4FileHandle h = hFile.get();
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4SetStringProperty(h, propName, value);
        }
        require(ok, "MP4SetStringProperty");
    }

    void MP4SetBytesProperty_wrapper(MP4FileHandleWrapper &hFile, const char *propName, const std::string &data)
    {
        MP4FileHandle h = hFile.get();
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4SetBytesProperty(h, propName, reinterpret_cast<const uint8_t *>(data.data()),
                                     static_cast<uint32_t>(data.size()));
        }
        require(ok, "MP4SetBytesProperty");
    }

    bool MP4HaveTrackAtom_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *atomName)
    {
        MP4FileHandle h = hFile.get();
        py::gil_scoped_release release;
        return MP4HaveTrackAtom(h, trackId, atomName);
    }

    std::string MP4GetTrackMediaDataName_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId)
    {
        MP4FileHandle h = hFile.get();
        const char *name;
        {
            py::gil_scoped_release release;
            name = MP4GetTrackMediaDataName(h, trackId);
        }
        if (name == nullptr)
        {
            throw MP4Error("MP4GetTrackMediaDataName failed");
        }
        return std::string(name);
    }

    MP4Duration MP4GetTrackFixedSampleDuration_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId)
    {
        MP4FileHandle h = hFile.get();
        py::gil_scoped_release release;
        return MP4GetTrackFixedSampleDuration(h, trackId);
    }

    uint32_t MP4GetTrackBitRate_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId)
    {
        MP4FileHandle h = hFile.get();
        py::gil_scoped_release release;
        return MP4GetTrackBitRate(h, trackId);
    }

    double MP4GetTrackVideoFrameRate_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId)
    {
        MP4FileHandle h = hFile.get();
        py::gil_scoped_release release;
        return MP4GetTrackVideoFrameRate(h, trackId);
    }

    int MP4GetTrackAudioChannels_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId)
    {
        MP4FileHandle h = hFile.get();
        int channels;
        {
            py::gil_scoped_release release;
            channels = MP4GetTrackAudioChannels(h, trackId);
        }
        if (channels < 0)
        {
            throw MP4Error("MP4GetTrackAudioChannels failed");
        }
        return channels;
    }

    std::string MP4GetTrackESConfiguration_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId)
    {
        MP4FileHandle h = hFile.get();
        uint8_t *bytes = nullptr;
        uint32_t size = 0;
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4GetTrackESConfiguration(h, trackId, &bytes, &size);
        }
        if (!ok)
        {
            if (bytes != nullptr)
            {
                MP4Free(bytes);
            }
            throw MP4Error("MP4GetTrackESConfiguration failed");
        }
        return take_allocated_bytes(bytes, size);
    }

    void MP4SetTrackESConfiguration_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const std::string &data)
    {
        MP4FileHandle h = hFile.get();
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4SetTrackESConfiguration(h, trackId, reinterpret_cast<const uint8_t *>(data.data()),
                                            static_cast<uint32_t>(data.size()));
        }
        require(ok, "MP4SetTrackESConfiguration");
    }

    uint64_t MP4GetTrackIntegerProperty_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *propName)
    {
        MP4FileHandle h = hFile.get();
        uint64_t value = 0;
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4GetTrackIntegerProperty(h, trackId, propName, &value);
        }
        require(ok, "MP4GetTrackIntegerProperty");
        return value;
    }

    float MP4GetTrackFloatProperty_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *propName)
    {
        MP4FileHandle h = hFile.get();
        float value = 0;
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4GetTrackFloatProperty(h, trackId, propName, &value);
        }
        require(ok, "MP4GetTrackFloatProperty");
        return value;
    }

    std::string MP4GetTrackStringProperty_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *propName)
    {
        MP4FileHandle h = hFile.get();
        const char *value = nullptr;
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4GetTrackStringProperty(h, trackId, propName, &value);
        }
        require(ok, "MP4GetTrackStringProperty");
        if (value == nullptr)
        {
            throw MP4Error("MP4GetTrackStringProperty failed");
        }
        return std::string(value);
    }

    std::string MP4GetTrackBytesProperty_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *propName)
    {
        MP4FileHandle h = hFile.get();
        uint8_t *bytes = nullptr;
        uint32_t size = 0;
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4GetTrackBytesProperty(h, trackId, propName, &bytes, &size);
        }
        if (!ok)
        {
            if (bytes != nullptr)
            {
                MP4Free(bytes);
            }
            throw MP4Error("MP4GetTrackBytesProperty failed");
        }
        return take_allocated_bytes(bytes, size);
    }

    void MP4SetTrackIntegerProperty_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *propName,
                                            int64_t value)
    {
        MP4FileHandle h = hFile.get();
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4SetTrackIntegerProperty(h, trackId, propName, value);
        }
        require(ok, "MP4SetTrackIntegerProperty");
    }

    void MP4SetTrackFloatProperty_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *propName,
                                          float value)
    {
        MP4FileHandle h = hFile.get();
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4SetTrackFloatProperty(h, trackId, propName, value);
        }
        require(ok, "MP4SetTrackFloatProperty");
    }

    void MP4SetTrackStringProperty_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *propName,
                                           const char *value)
    {
        MP4FileHandle h = hFile.get();
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4SetTrackStringProperty(h, trackId, propName, value);
        }
        require(ok, "MP4SetTrackStringProperty");
    }

    void MP4SetTrackBytesProperty_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *propName,
                                          const std::string &data)
    {
        MP4FileHandle h = hFile.get();
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4SetTrackBytesProperty(h, trackId, propName, reinterpret_cast<const uint8_t *>(data.data()),
                                          static_cast<uint32_t>(data.size()));
        }
        require(ok, "MP4SetTrackBytesProperty");
    }

    MP4SampleData MP4ReadSample_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, MP4SampleId sampleId)
    {
        MP4FileHandle h = hFile.get();
        uint8_t *bytes = nullptr;
        uint32_t numBytes = 0;
        MP4Timestamp startTime = 0;
        MP4Duration duration = 0;
        MP4Duration renderingOffset = 0;
        bool isSyncSample = false;
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4ReadSample(h, trackId, sampleId, &bytes, &numBytes, &startTime, &duration, &renderingOffset,
                               &isSyncSample);
        }
        return finish_read_sample(ok, "MP4ReadSample", bytes, numBytes, startTime, duration, renderingOffset,
                                  isSyncSample);
    }

    MP4SampleData MP4ReadSampleFromTime_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, MP4Timestamp when)
    {
        MP4FileHandle h = hFile.get();
        uint8_t *bytes = nullptr;
        uint32_t numBytes = 0;
        MP4Timestamp startTime = 0;
        MP4Duration duration = 0;
        MP4Duration renderingOffset = 0;
        bool isSyncSample = false;
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4ReadSampleFromTime(h, trackId, when, &bytes, &numBytes, &startTime, &duration, &renderingOffset,
                                       &isSyncSample);
        }
        return finish_read_sample(ok, "MP4ReadSampleFromTime", bytes, numBytes, startTime, duration, renderingOffset,
                                  isSyncSample);
    }

    void MP4WriteSample_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, const std::string &data,
                                MP4Duration duration, MP4Duration renderingOffset, bool isSyncSample)
    {
        MP4FileHandle h = hFile.get();
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4WriteSample(h, trackId, reinterpret_cast<const uint8_t *>(data.data()),
                                static_cast<uint32_t>(data.size()), duration, renderingOffset, isSyncSample);
        }
        require(ok, "MP4WriteSample");
    }

    void MP4CopySample_wrapper(MP4FileHandleWrapper &srcFile, MP4TrackId srcTrackId, MP4SampleId srcSampleId,
                               MP4FileHandleWrapper *dstFile, MP4TrackId dstTrackId, MP4Duration dstSampleDuration)
    {
        MP4FileHandle src = srcFile.get();
        MP4FileHandle dst = dstFile == nullptr ? MP4_INVALID_FILE_HANDLE : dstFile->get();
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4CopySample(src, srcTrackId, srcSampleId, dst, dstTrackId, dstSampleDuration);
        }
        require(ok, "MP4CopySample");
    }

    uint32_t MP4GetSampleSize_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, MP4SampleId sampleId)
    {
        MP4FileHandle h = hFile.get();
        py::gil_scoped_release release;
        return MP4GetSampleSize(h, trackId, sampleId);
    }

    MP4SampleId MP4GetSampleIdFromTime_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, MP4Timestamp when,
                                               bool wantSyncSample)
    {
        MP4FileHandle h = hFile.get();
        MP4SampleId id;
        {
            py::gil_scoped_release release;
            id = MP4GetSampleIdFromTime(h, trackId, when, wantSyncSample);
        }
        return require_sample(id, "MP4GetSampleIdFromTime");
    }

    MP4Timestamp MP4GetSampleTime_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, MP4SampleId sampleId)
    {
        MP4FileHandle h = hFile.get();
        MP4Timestamp when;
        {
            py::gil_scoped_release release;
            when = MP4GetSampleTime(h, trackId, sampleId);
        }
        if (when == MP4_INVALID_TIMESTAMP)
        {
            throw MP4Error("MP4GetSampleTime failed");
        }
        return when;
    }

    MP4Duration MP4GetSampleDuration_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, MP4SampleId sampleId)
    {
        MP4FileHandle h = hFile.get();
        MP4Duration duration;
        {
            py::gil_scoped_release release;
            duration = MP4GetSampleDuration(h, trackId, sampleId);
        }
        if (duration == MP4_INVALID_DURATION)
        {
            throw MP4Error("MP4GetSampleDuration failed");
        }
        return duration;
    }

    bool MP4GetSampleSync_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId, MP4SampleId sampleId)
    {
        MP4FileHandle h = hFile.get();
        int8_t sync;
        {
            py::gil_scoped_release release;
            sync = MP4GetSampleSync(h, trackId, sampleId);
        }
        if (sync < 0)
        {
            throw MP4Error("MP4GetSampleSync failed");
        }
        return sync != 0;
    }

    MP4SampleId MP4GetTrackNumberOfSamples_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId)
    {
        MP4FileHandle h = hFile.get();
        py::gil_scoped_release release;
        return MP4GetTrackNumberOfSamples(h, trackId);
    }

    uint32_t MP4GetTrackMaxSampleSize_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId trackId)
    {
        MP4FileHandle h = hFile.get();
        py::gil_scoped_release release;
        return MP4GetTrackMaxSampleSize(h, trackId);
    }

    void MP4TagsFetch_wrapper(MP4TagsWrapper &tags, MP4FileHandleWrapper &hFile)
    {
        const MP4Tags *t = tags.get();
        MP4FileHandle h = hFile.get();
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4TagsFetch(t, h);
        }
        require(ok, "MP4TagsFetch");
    }

    void MP4TagsStore_wrapper(MP4TagsWrapper &tags, MP4FileHandleWrapper &hFile)
    {
        const MP4Tags *t = tags.get();
        MP4FileHandle h = hFile.get();
        bool ok;
        {
            py::gil_scoped_release release;
            ok = MP4TagsStore(t, h);
        }
        require(ok, "MP4TagsStore");
    }

    bool MP4TagsHasMetadata_wrapper(MP4TagsWrapper &tags)
    {
        bool has = false;
        require(MP4TagsHasMetadata(tags.get(), &has), "MP4TagsHasMetadata");
        return has;
    }

    void MP4TagsSetString_wrapper(MP4TagsWrapper &tags, bool (*setter)(const MP4Tags *, const char *),
                                  const std::optional<std::string> &value, const char *what)
    {
        require(setter(tags.get(), optional_cstr(value)), what);
    }

    void MP4TagsAddArtwork_wrapper(MP4TagsWrapper &tags, const std::string &data, MP4TagArtworkType type)
    {
        MP4TagArtwork art{};
        art.data = const_cast<void *>(static_cast<const void *>(data.data()));
        art.size = static_cast<uint32_t>(data.size());
        art.type = type;
        require(MP4TagsAddArtwork(tags.get(), &art), "MP4TagsAddArtwork");
    }

    void MP4TagsSetArtwork_wrapper(MP4TagsWrapper &tags, uint32_t index, const std::string &data,
                                   MP4TagArtworkType type)
    {
        MP4TagArtwork art{};
        art.data = const_cast<void *>(static_cast<const void *>(data.data()));
        art.size = static_cast<uint32_t>(data.size());
        art.type = type;
        require(MP4TagsSetArtwork(tags.get(), index, &art), "MP4TagsSetArtwork");
    }

    void MP4TagsRemoveArtwork_wrapper(MP4TagsWrapper &tags, uint32_t index)
    {
        require(MP4TagsRemoveArtwork(tags.get(), index), "MP4TagsRemoveArtwork");
    }

    namespace
    {
        void copy_chapter_title(char *dest, const std::string &title)
        {
            if (title.size() > MP4V2_CHAPTER_TITLE_MAX)
            {
                throw MP4Error("chapter title exceeds MP4V2_CHAPTER_TITLE_MAX");
            }
            std::memset(dest, 0, MP4V2_CHAPTER_TITLE_MAX + 1);
            if (!title.empty())
            {
                std::memcpy(dest, title.c_str(), title.size());
            }
        }
    } // namespace

    MP4TrackId MP4AddChapterTextTrack_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId refTrackId, uint32_t timescale)
    {
        MP4FileHandle h = hFile.get();
        MP4TrackId id;
        {
            py::gil_scoped_release release;
            id = MP4AddChapterTextTrack(h, refTrackId, timescale);
        }
        return require_track(id, "MP4AddChapterTextTrack");
    }

    void MP4AddChapter_wrapper(MP4FileHandleWrapper &hFile, MP4TrackId chapterTrackId, MP4Duration chapterDuration,
                               const std::optional<std::string> &chapterTitle)
    {
        MP4FileHandle h = hFile.get();
        const char *title = optional_cstr(chapterTitle);
        py::gil_scoped_release release;
        MP4AddChapter(h, chapterTrackId, chapterDuration, title);
    }

    void MP4AddNeroChapter_wrapper(MP4FileHandleWrapper &hFile, MP4Timestamp chapterStart,
                                   const std::optional<std::string> &chapterTitle)
    {
        MP4FileHandle h = hFile.get();
        const char *title = optional_cstr(chapterTitle);
        py::gil_scoped_release release;
        MP4AddNeroChapter(h, chapterStart, title);
    }

    MP4ChapterType MP4ConvertChapters_wrapper(MP4FileHandleWrapper &hFile, MP4ChapterType toChapterType)
    {
        MP4FileHandle h = hFile.get();
        py::gil_scoped_release release;
        return MP4ConvertChapters(h, toChapterType);
    }

    MP4ChapterType MP4DeleteChapters_wrapper(MP4FileHandleWrapper &hFile, MP4ChapterType chapterType,
                                             MP4TrackId chapterTrackId)
    {
        MP4FileHandle h = hFile.get();
        py::gil_scoped_release release;
        return MP4DeleteChapters(h, chapterType, chapterTrackId);
    }

    MP4ChapterListResult MP4GetChapters_wrapper(MP4FileHandleWrapper &hFile, MP4ChapterType chapterType)
    {
        MP4FileHandle h = hFile.get();
        MP4Chapter_t *list = nullptr;
        uint32_t count = 0;
        MP4ChapterType type;
        {
            py::gil_scoped_release release;
            type = MP4GetChapters(h, &list, &count, chapterType);
        }
        MP4ChapterListResult result;
        result.type = type;
        if (list != nullptr)
        {
            result.chapters.reserve(count);
            for (uint32_t i = 0; i < count; ++i)
            {
                MP4ChapterData item;
                item.duration = list[i].duration;
                item.title = list[i].title;
                result.chapters.push_back(std::move(item));
            }
            MP4Free(list);
        }
        return result;
    }

    MP4ChapterType MP4SetChapters_wrapper(MP4FileHandleWrapper &hFile, const std::vector<MP4ChapterData> &chapters,
                                          MP4ChapterType chapterType)
    {
        MP4FileHandle h = hFile.get();
        std::vector<MP4Chapter_t> native(chapters.size());
        for (size_t i = 0; i < chapters.size(); ++i)
        {
            native[i].duration = chapters[i].duration;
            copy_chapter_title(native[i].title, chapters[i].title);
        }
        MP4ChapterType written;
        {
            py::gil_scoped_release release;
            written = MP4SetChapters(h, native.empty() ? nullptr : native.data(),
                                     static_cast<uint32_t>(native.size()), chapterType);
        }
        if (!chapters.empty() && written == MP4ChapterTypeNone)
        {
            throw MP4Error("MP4SetChapters failed");
        }
        return written;
    }
} // namespace raw
