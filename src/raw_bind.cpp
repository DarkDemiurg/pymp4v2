#include "pymp4v2/error.h"
#include "pymp4v2/raw.h"

namespace py = pybind11;

namespace raw
{
    namespace
    {
        void bind_constants(py::module_ &m_raw)
        {
            m_raw.attr("MP4_INVALID_SAMPLE_ID") = py::cast(MP4_INVALID_SAMPLE_ID);
            m_raw.attr("MP4_INVALID_TIMESTAMP") = py::cast(MP4_INVALID_TIMESTAMP);
            m_raw.attr("MP4_INVALID_DURATION") = py::cast(MP4_INVALID_DURATION);
            m_raw.attr("MP4_INVALID_EDIT_ID") = py::cast(MP4_INVALID_EDIT_ID);
            m_raw.attr("MP4_OD_TRACK_TYPE") = MP4_OD_TRACK_TYPE;
            m_raw.attr("MP4_SCENE_TRACK_TYPE") = MP4_SCENE_TRACK_TYPE;
            m_raw.attr("MP4_AUDIO_TRACK_TYPE") = MP4_AUDIO_TRACK_TYPE;
            m_raw.attr("MP4_VIDEO_TRACK_TYPE") = MP4_VIDEO_TRACK_TYPE;
            m_raw.attr("MP4_HINT_TRACK_TYPE") = MP4_HINT_TRACK_TYPE;
            m_raw.attr("MP4_CNTL_TRACK_TYPE") = MP4_CNTL_TRACK_TYPE;
            m_raw.attr("MP4_TEXT_TRACK_TYPE") = MP4_TEXT_TRACK_TYPE;
            m_raw.attr("MP4_SUBTITLE_TRACK_TYPE") = MP4_SUBTITLE_TRACK_TYPE;
            m_raw.attr("MP4_SUBPIC_TRACK_TYPE") = MP4_SUBPIC_TRACK_TYPE;
            m_raw.attr("MP4_CLOCK_TRACK_TYPE") = MP4_CLOCK_TRACK_TYPE;
            m_raw.attr("MP4_MPEG7_TRACK_TYPE") = MP4_MPEG7_TRACK_TYPE;
            m_raw.attr("MP4_OCI_TRACK_TYPE") = MP4_OCI_TRACK_TYPE;
            m_raw.attr("MP4_IPMP_TRACK_TYPE") = MP4_IPMP_TRACK_TYPE;
            m_raw.attr("MP4_MPEGJ_TRACK_TYPE") = MP4_MPEGJ_TRACK_TYPE;
            m_raw.attr("MP4_MPEG4_AUDIO_TYPE") = py::int_(MP4_MPEG4_AUDIO_TYPE);
            m_raw.attr("MP4_MPEG4_VIDEO_TYPE") = py::int_(MP4_MPEG4_VIDEO_TYPE);
        }

        void bind_tracks(py::module_ &m_raw)
        {
            m_raw.def("MP4GetNumberOfTracks", &MP4GetNumberOfTracks_wrapper, py::arg("hFile"),
                      py::arg("type") = py::none(), py::arg("subType") = 0,
                      R"doc(
    Number of tracks of the given type (any type if type is None).
)doc");

            m_raw.def("MP4FindTrackId", &MP4FindTrackId_wrapper, py::arg("hFile"), py::arg("index"),
                      py::arg("type") = py::none(), py::arg("subType") = 0,
                      R"doc(
    Track id of the index'th track matching type (0-based). Raises MP4Error if none.
)doc");

            m_raw.def("MP4GetTrackType", &MP4GetTrackType_wrapper, py::arg("hFile"), py::arg("trackId"),
                      R"doc(
    Four-character track type (e.g. MP4_VIDEO_TRACK_TYPE). Raises MP4Error on failure.
)doc");

            m_raw.def("MP4DeleteTrack", &MP4DeleteTrack_wrapper, py::arg("hFile"), py::arg("trackId"),
                      R"doc(
    Delete track control info. Sample data is removed later by MP4Optimize.
)doc");

            m_raw.def("MP4AddAudioTrack", &MP4AddAudioTrack_wrapper, py::arg("hFile"), py::arg("timeScale"),
                      py::arg("sampleDuration"), py::arg("audioType") = MP4_MPEG4_AUDIO_TYPE,
                      R"doc(
    Add an audio track. sampleDuration may be MP4_INVALID_DURATION for variable duration.
)doc");

            m_raw.def("MP4AddVideoTrack", &MP4AddVideoTrack_wrapper, py::arg("hFile"), py::arg("timeScale"),
                      py::arg("sampleDuration"), py::arg("width"), py::arg("height"),
                      py::arg("videoType") = MP4_MPEG4_VIDEO_TYPE,
                      R"doc(
    Add a video track with the given pixel size.
)doc");
        }

        void bind_props(py::module_ &m_raw)
        {
            m_raw.def("MP4HaveAtom", &MP4HaveAtom_wrapper, py::arg("hFile"), py::arg("atomName"),
                      "True if the atom path exists (predicate; does not raise on absence).");
            m_raw.def("MP4GetDuration", &MP4GetDuration_wrapper, py::arg("hFile"),
                      "Movie duration in movie timescale units.");
            m_raw.def("MP4GetTimeScale", &MP4GetTimeScale_wrapper, py::arg("hFile"),
                      "Movie timescale (ticks per second).");
            m_raw.def("MP4SetTimeScale", &MP4SetTimeScale_wrapper, py::arg("hFile"), py::arg("value"));
            m_raw.def("MP4GetTrackDuration", &MP4GetTrackDuration_wrapper, py::arg("hFile"), py::arg("trackId"),
                      "Track duration in track timescale units.");
            m_raw.def("MP4GetTrackTimeScale", &MP4GetTrackTimeScale_wrapper, py::arg("hFile"), py::arg("trackId"));
            m_raw.def("MP4SetTrackTimeScale", &MP4SetTrackTimeScale_wrapper, py::arg("hFile"), py::arg("trackId"),
                      py::arg("value"));
            m_raw.def("MP4GetTrackVideoWidth", &MP4GetTrackVideoWidth_wrapper, py::arg("hFile"), py::arg("trackId"));
            m_raw.def("MP4GetTrackVideoHeight", &MP4GetTrackVideoHeight_wrapper, py::arg("hFile"), py::arg("trackId"));
            m_raw.def("MP4GetTrackLanguage", &MP4GetTrackLanguage_wrapper, py::arg("hFile"), py::arg("trackId"),
                      "ISO-639-2/T 3-letter language code.");
            m_raw.def("MP4SetTrackLanguage", &MP4SetTrackLanguage_wrapper, py::arg("hFile"), py::arg("trackId"),
                      py::arg("code"));
            m_raw.def("MP4GetTrackName", &MP4GetTrackName_wrapper, py::arg("hFile"), py::arg("trackId"));
            m_raw.def("MP4SetTrackName", &MP4SetTrackName_wrapper, py::arg("hFile"), py::arg("trackId"),
                      py::arg("name"));
            m_raw.def("MP4ChangeMovieTimeScale", &MP4ChangeMovieTimeScale_wrapper, py::arg("hFile"), py::arg("value"),
                      "Recalculate movie timescale-dependent fields in moov.mvhd.");
            m_raw.def("MP4GetIntegerProperty", &MP4GetIntegerProperty_wrapper, py::arg("hFile"), py::arg("propName"),
                      "Integer atom property, e.g. moov.mvhd.timescale.");
            m_raw.def("MP4GetFloatProperty", &MP4GetFloatProperty_wrapper, py::arg("hFile"), py::arg("propName"));
            m_raw.def("MP4GetStringProperty", &MP4GetStringProperty_wrapper, py::arg("hFile"), py::arg("propName"),
                      "String atom property, e.g. ftyp.majorBrand. Library-owned pointer is copied.");
            m_raw.def(
                "MP4GetBytesProperty",
                [](MP4FileHandleWrapper &hFile, const char *propName)
                { return py::bytes(MP4GetBytesProperty_wrapper(hFile, propName)); },
                py::arg("hFile"), py::arg("propName"),
                "Bytes atom property. Wrapper copies and MP4Free's the C buffer.");
            m_raw.def("MP4SetIntegerProperty", &MP4SetIntegerProperty_wrapper, py::arg("hFile"), py::arg("propName"),
                      py::arg("value"));
            m_raw.def("MP4SetFloatProperty", &MP4SetFloatProperty_wrapper, py::arg("hFile"), py::arg("propName"),
                      py::arg("value"));
            m_raw.def("MP4SetStringProperty", &MP4SetStringProperty_wrapper, py::arg("hFile"), py::arg("propName"),
                      py::arg("value"));
            m_raw.def("MP4SetBytesProperty", &MP4SetBytesProperty_wrapper, py::arg("hFile"), py::arg("propName"),
                      py::arg("pValue"));
            m_raw.def("MP4HaveTrackAtom", &MP4HaveTrackAtom_wrapper, py::arg("hFile"), py::arg("trackId"),
                      py::arg("atomName"),
                      "True if the track atom path exists (predicate; does not raise on absence).");
            m_raw.def("MP4GetTrackMediaDataName", &MP4GetTrackMediaDataName_wrapper, py::arg("hFile"),
                      py::arg("trackId"), "Four-character sample-entry type (stsd child).");
            m_raw.def("MP4GetTrackFixedSampleDuration", &MP4GetTrackFixedSampleDuration_wrapper, py::arg("hFile"),
                      py::arg("trackId"),
                      "Fixed sample duration, or MP4_INVALID_DURATION if samples vary.");
            m_raw.def("MP4GetTrackBitRate", &MP4GetTrackBitRate_wrapper, py::arg("hFile"), py::arg("trackId"),
                      "Average bitrate in bits per second (0 if unknown).");
            m_raw.def("MP4GetTrackVideoFrameRate", &MP4GetTrackVideoFrameRate_wrapper, py::arg("hFile"),
                      py::arg("trackId"), "Average frames per second (0 if unknown).");
            m_raw.def("MP4GetTrackAudioChannels", &MP4GetTrackAudioChannels_wrapper, py::arg("hFile"),
                      py::arg("trackId"), "Channel count. Raises MP4Error if the track has no audio channels.");
            m_raw.def(
                "MP4GetTrackESConfiguration",
                [](MP4FileHandleWrapper &hFile, MP4TrackId trackId)
                { return py::bytes(MP4GetTrackESConfiguration_wrapper(hFile, trackId)); },
                py::arg("hFile"), py::arg("trackId"),
                "Elementary-stream config as bytes. Wrapper copies and MP4Free's the C buffer.");
            m_raw.def("MP4SetTrackESConfiguration", &MP4SetTrackESConfiguration_wrapper, py::arg("hFile"),
                      py::arg("trackId"), py::arg("pConfig"));
            m_raw.def("MP4GetTrackIntegerProperty", &MP4GetTrackIntegerProperty_wrapper, py::arg("hFile"),
                      py::arg("trackId"), py::arg("propName"),
                      "Track-scoped integer property, e.g. tkhd.layer.");
            m_raw.def("MP4GetTrackFloatProperty", &MP4GetTrackFloatProperty_wrapper, py::arg("hFile"),
                      py::arg("trackId"), py::arg("propName"));
            m_raw.def("MP4GetTrackStringProperty", &MP4GetTrackStringProperty_wrapper, py::arg("hFile"),
                      py::arg("trackId"), py::arg("propName"));
            m_raw.def(
                "MP4GetTrackBytesProperty",
                [](MP4FileHandleWrapper &hFile, MP4TrackId trackId, const char *propName)
                { return py::bytes(MP4GetTrackBytesProperty_wrapper(hFile, trackId, propName)); },
                py::arg("hFile"), py::arg("trackId"), py::arg("propName"),
                "Track-scoped bytes property. Wrapper copies and MP4Free's the C buffer.");
            m_raw.def("MP4SetTrackIntegerProperty", &MP4SetTrackIntegerProperty_wrapper, py::arg("hFile"),
                      py::arg("trackId"), py::arg("propName"), py::arg("value"));
            m_raw.def("MP4SetTrackFloatProperty", &MP4SetTrackFloatProperty_wrapper, py::arg("hFile"),
                      py::arg("trackId"), py::arg("propName"), py::arg("value"));
            m_raw.def("MP4SetTrackStringProperty", &MP4SetTrackStringProperty_wrapper, py::arg("hFile"),
                      py::arg("trackId"), py::arg("propName"), py::arg("value"));
            m_raw.def("MP4SetTrackBytesProperty", &MP4SetTrackBytesProperty_wrapper, py::arg("hFile"),
                      py::arg("trackId"), py::arg("propName"), py::arg("pValue"));
        }

        void bind_samples(py::module_ &m_raw)
        {
            py::class_<MP4SampleData>(m_raw, "MP4Sample",
                                      "One sample from MP4ReadSample: data (bytes) plus timing fields.")
                .def_property_readonly("data", [](const MP4SampleData &s)
                                       { return py::bytes(s.data); })
                .def_readonly("startTime", &MP4SampleData::startTime)
                .def_readonly("duration", &MP4SampleData::duration)
                .def_readonly("renderingOffset", &MP4SampleData::renderingOffset)
                .def_readonly("isSyncSample", &MP4SampleData::isSyncSample)
                .def("__iter__",
                     [](const MP4SampleData &s)
                     {
                         return py::iter(py::make_tuple(py::bytes(s.data), s.startTime, s.duration, s.renderingOffset,
                                                        s.isSyncSample));
                     });

            m_raw.def("MP4ReadSample", &MP4ReadSample_wrapper, py::arg("hFile"), py::arg("trackId"), py::arg("sampleId"),
                      R"doc(
    Read sample ``sampleId`` (first sample is 1). Returns MP4Sample.
    Unpacks as ``(data, startTime, duration, renderingOffset, isSyncSample)``.
    Samples written on a create/modify handle are readable after ``MP4Close``.
)doc");
            m_raw.def("MP4ReadSampleFromTime", &MP4ReadSampleFromTime_wrapper, py::arg("hFile"), py::arg("trackId"),
                      py::arg("when"),
                      R"doc(
    Read the sample that contains ``when`` (track timescale). Returns MP4Sample.
)doc");
            m_raw.def("MP4WriteSample", &MP4WriteSample_wrapper, py::arg("hFile"), py::arg("trackId"), py::arg("pBytes"),
                      py::arg("duration") = MP4_INVALID_DURATION, py::arg("renderingOffset") = 0,
                      py::arg("isSyncSample") = true,
                      R"doc(
    Append a sample. ``pBytes`` is bytes. duration defaults to MP4_INVALID_DURATION.
)doc");
            m_raw.def("MP4CopySample", &MP4CopySample_wrapper, py::arg("srcFile"), py::arg("srcTrackId"),
                      py::arg("srcSampleId"), py::arg("dstFile") = static_cast<MP4FileHandleWrapper *>(nullptr),
                      py::arg("dstTrackId") = MP4_INVALID_TRACK_ID,
                      py::arg("dstSampleDuration") = MP4_INVALID_DURATION,
                      R"doc(
    Copy a sample. ``dstFile`` None (or the source handle) copies in the same file.
    ``dstTrackId`` MP4_INVALID_TRACK_ID copies onto the source track.
    ``dstSampleDuration`` MP4_INVALID_DURATION keeps the source duration.
)doc");
            m_raw.def("MP4GetSampleSize", &MP4GetSampleSize_wrapper, py::arg("hFile"), py::arg("trackId"),
                      py::arg("sampleId"));
            m_raw.def("MP4GetSampleIdFromTime", &MP4GetSampleIdFromTime_wrapper, py::arg("hFile"), py::arg("trackId"),
                      py::arg("when"), py::arg("wantSyncSample") = false,
                      "Sample id containing ``when`` (track timescale). Raises if none.");
            m_raw.def("MP4GetSampleTime", &MP4GetSampleTime_wrapper, py::arg("hFile"), py::arg("trackId"),
                      py::arg("sampleId"), "Sample start time in the track timescale.");
            m_raw.def("MP4GetSampleDuration", &MP4GetSampleDuration_wrapper, py::arg("hFile"), py::arg("trackId"),
                      py::arg("sampleId"), "Sample duration in the track timescale.");
            m_raw.def("MP4GetSampleSync", &MP4GetSampleSync_wrapper, py::arg("hFile"), py::arg("trackId"),
                      py::arg("sampleId"), "True if the sample is a sync/random-access point.");
            m_raw.def("MP4GetTrackNumberOfSamples", &MP4GetTrackNumberOfSamples_wrapper, py::arg("hFile"),
                      py::arg("trackId"));
            m_raw.def("MP4GetTrackMaxSampleSize", &MP4GetTrackMaxSampleSize_wrapper, py::arg("hFile"),
                      py::arg("trackId"));
        }

        std::optional<std::string> tag_str(const char *value)
        {
            if (value == nullptr)
            {
                return std::nullopt;
            }
            return std::string(value);
        }

        void bind_tag_string(py::module_ &m_raw, py::class_<MP4TagsWrapper> &cls, const char *field,
                             const char *(*getter)(const MP4Tags *),
                             bool (*setter)(const MP4Tags *, const char *), const char *setter_name)
        {
            cls.def_property_readonly(field, [getter](const MP4TagsWrapper &self)
                                      { return tag_str(getter(self.get())); });
            m_raw.def(
                setter_name,
                [setter, setter_name](MP4TagsWrapper &tags, const std::optional<std::string> &value)
                {
                    MP4TagsSetString_wrapper(tags, setter, value, setter_name);
                },
                py::arg("tags"), py::arg("value") = py::none());
        }

        template <typename T>
        void bind_tag_scalar(py::module_ &m_raw, py::class_<MP4TagsWrapper> &cls, const char *field,
                             const T *(*getter)(const MP4Tags *), bool (*setter)(const MP4Tags *, const T *),
                             const char *setter_name)
        {
            cls.def_property_readonly(field, [getter](const MP4TagsWrapper &self) -> std::optional<T>
                                      {
                                          const T *p = getter(self.get());
                                          if (p == nullptr)
                                          {
                                              return std::nullopt;
                                          }
                                          return *p;
                                      });
            m_raw.def(
                setter_name,
                [setter, setter_name](MP4TagsWrapper &tags, const std::optional<T> &value)
                {
                    MP4TagsSetScalar_wrapper(tags, setter, value, setter_name);
                },
                py::arg("tags"), py::arg("value") = py::none());
        }

        struct PyMP4TagArtwork
        {
            std::string data;
            MP4TagArtworkType type;
        };

        void bind_tags(py::module_ &m_raw)
        {
            py::class_<PyMP4TagArtwork>(m_raw, "MP4TagArtwork",
                                        "Artwork item: ``data`` is a copy as bytes, ``type`` is MP4TagArtworkType.")
                .def_property_readonly("data", [](const PyMP4TagArtwork &self)
                                       { return py::bytes(self.data); })
                .def_property_readonly("size", [](const PyMP4TagArtwork &self)
                                       { return static_cast<uint32_t>(self.data.size()); })
                .def_readonly("type", &PyMP4TagArtwork::type);

            py::class_<MP4TagTrack>(m_raw, "MP4TagTrack")
                .def(py::init<>())
                .def(py::init([](uint16_t index, uint16_t total)
                              {
                                  MP4TagTrack t{};
                                  t.index = index;
                                  t.total = total;
                                  return t;
                              }),
                     py::arg("index"), py::arg("total"))
                .def_readwrite("index", &MP4TagTrack::index)
                .def_readwrite("total", &MP4TagTrack::total);

            py::class_<MP4TagDisk>(m_raw, "MP4TagDisk")
                .def(py::init<>())
                .def(py::init([](uint16_t index, uint16_t total)
                              {
                                  MP4TagDisk d{};
                                  d.index = index;
                                  d.total = total;
                                  return d;
                              }),
                     py::arg("index"), py::arg("total"))
                .def_readwrite("index", &MP4TagDisk::index)
                .def_readwrite("total", &MP4TagDisk::total);

            auto cls = py::class_<MP4TagsWrapper>(m_raw, "MP4Tags",
                                                  "iTMF tags snapshot. Allocate with MP4TagsAlloc(); free with "
                                                  "MP4TagsFree() or the destructor.");
            cls.def(py::init<>())
                .def("is_valid", &MP4TagsWrapper::is_valid)
                .def("__enter__", [](MP4TagsWrapper &self) -> MP4TagsWrapper &
                     { return self; })
                .def("__exit__",
                     [](MP4TagsWrapper &self, const py::object &, const py::object &, const py::object &)
                     {
                         self.free();
                         return false;
                     });

            cls.def_property_readonly(
                "artwork",
                [](const MP4TagsWrapper &self)
                {
                    const MP4Tags *t = self.get();
                    py::list out;
                    for (uint32_t i = 0; i < t->artworkCount; ++i)
                    {
                        const MP4TagArtwork &a = t->artwork[i];
                        PyMP4TagArtwork item;
                        if (a.data != nullptr && a.size > 0)
                        {
                            item.data.assign(static_cast<const char *>(a.data), a.size);
                        }
                        item.type = a.type;
                        out.append(item);
                    }
                    return out;
                });
            cls.def_property_readonly("artworkCount", [](const MP4TagsWrapper &self)
                                      { return self.get()->artworkCount; });
            cls.def_property_readonly(
                "track",
                [](const MP4TagsWrapper &self) -> std::optional<MP4TagTrack>
                {
                    if (self.get()->track == nullptr)
                    {
                        return std::nullopt;
                    }
                    return *self.get()->track;
                });
            cls.def_property_readonly(
                "disk",
                [](const MP4TagsWrapper &self) -> std::optional<MP4TagDisk>
                {
                    if (self.get()->disk == nullptr)
                    {
                        return std::nullopt;
                    }
                    return *self.get()->disk;
                });
            cls.def_property_readonly(
                "tempo",
                [](const MP4TagsWrapper &self) -> std::optional<uint16_t>
                {
                    if (self.get()->tempo == nullptr)
                    {
                        return std::nullopt;
                    }
                    return *self.get()->tempo;
                });

            bind_tag_string(m_raw, cls, "name", [](const MP4Tags *t)
                            { return t->name; }, MP4TagsSetName, "MP4TagsSetName");
            bind_tag_string(m_raw, cls, "artist", [](const MP4Tags *t)
                            { return t->artist; }, MP4TagsSetArtist, "MP4TagsSetArtist");
            bind_tag_string(m_raw, cls, "albumArtist", [](const MP4Tags *t)
                            { return t->albumArtist; }, MP4TagsSetAlbumArtist, "MP4TagsSetAlbumArtist");
            bind_tag_string(m_raw, cls, "album", [](const MP4Tags *t)
                            { return t->album; }, MP4TagsSetAlbum, "MP4TagsSetAlbum");
            bind_tag_string(m_raw, cls, "grouping", [](const MP4Tags *t)
                            { return t->grouping; }, MP4TagsSetGrouping, "MP4TagsSetGrouping");
            bind_tag_string(m_raw, cls, "composer", [](const MP4Tags *t)
                            { return t->composer; }, MP4TagsSetComposer, "MP4TagsSetComposer");
            bind_tag_string(m_raw, cls, "comments", [](const MP4Tags *t)
                            { return t->comments; }, MP4TagsSetComments, "MP4TagsSetComments");
            bind_tag_string(m_raw, cls, "genre", [](const MP4Tags *t)
                            { return t->genre; }, MP4TagsSetGenre, "MP4TagsSetGenre");
            bind_tag_string(m_raw, cls, "releaseDate", [](const MP4Tags *t)
                            { return t->releaseDate; }, MP4TagsSetReleaseDate, "MP4TagsSetReleaseDate");
            bind_tag_string(m_raw, cls, "tvShow", [](const MP4Tags *t)
                            { return t->tvShow; }, MP4TagsSetTVShow, "MP4TagsSetTVShow");
            bind_tag_string(m_raw, cls, "tvNetwork", [](const MP4Tags *t)
                            { return t->tvNetwork; }, MP4TagsSetTVNetwork, "MP4TagsSetTVNetwork");
            bind_tag_string(m_raw, cls, "tvEpisodeID", [](const MP4Tags *t)
                            { return t->tvEpisodeID; }, MP4TagsSetTVEpisodeID, "MP4TagsSetTVEpisodeID");
            bind_tag_string(m_raw, cls, "description", [](const MP4Tags *t)
                            { return t->description; }, MP4TagsSetDescription, "MP4TagsSetDescription");
            bind_tag_string(m_raw, cls, "longDescription", [](const MP4Tags *t)
                            { return t->longDescription; }, MP4TagsSetLongDescription, "MP4TagsSetLongDescription");
            bind_tag_string(m_raw, cls, "lyrics", [](const MP4Tags *t)
                            { return t->lyrics; }, MP4TagsSetLyrics, "MP4TagsSetLyrics");
            bind_tag_string(m_raw, cls, "sortName", [](const MP4Tags *t)
                            { return t->sortName; }, MP4TagsSetSortName, "MP4TagsSetSortName");
            bind_tag_string(m_raw, cls, "sortArtist", [](const MP4Tags *t)
                            { return t->sortArtist; }, MP4TagsSetSortArtist, "MP4TagsSetSortArtist");
            bind_tag_string(m_raw, cls, "sortAlbumArtist", [](const MP4Tags *t)
                            { return t->sortAlbumArtist; }, MP4TagsSetSortAlbumArtist, "MP4TagsSetSortAlbumArtist");
            bind_tag_string(m_raw, cls, "sortAlbum", [](const MP4Tags *t)
                            { return t->sortAlbum; }, MP4TagsSetSortAlbum, "MP4TagsSetSortAlbum");
            bind_tag_string(m_raw, cls, "sortComposer", [](const MP4Tags *t)
                            { return t->sortComposer; }, MP4TagsSetSortComposer, "MP4TagsSetSortComposer");
            bind_tag_string(m_raw, cls, "sortTVShow", [](const MP4Tags *t)
                            { return t->sortTVShow; }, MP4TagsSetSortTVShow, "MP4TagsSetSortTVShow");
            bind_tag_string(m_raw, cls, "copyright", [](const MP4Tags *t)
                            { return t->copyright; }, MP4TagsSetCopyright, "MP4TagsSetCopyright");
            bind_tag_string(m_raw, cls, "encodingTool", [](const MP4Tags *t)
                            { return t->encodingTool; }, MP4TagsSetEncodingTool, "MP4TagsSetEncodingTool");
            bind_tag_string(m_raw, cls, "encodedBy", [](const MP4Tags *t)
                            { return t->encodedBy; }, MP4TagsSetEncodedBy, "MP4TagsSetEncodedBy");
            bind_tag_string(m_raw, cls, "purchaseDate", [](const MP4Tags *t)
                            { return t->purchaseDate; }, MP4TagsSetPurchaseDate, "MP4TagsSetPurchaseDate");
            bind_tag_string(m_raw, cls, "keywords", [](const MP4Tags *t)
                            { return t->keywords; }, MP4TagsSetKeywords, "MP4TagsSetKeywords");
            bind_tag_string(m_raw, cls, "category", [](const MP4Tags *t)
                            { return t->category; }, MP4TagsSetCategory, "MP4TagsSetCategory");
            bind_tag_string(m_raw, cls, "iTunesAccount", [](const MP4Tags *t)
                            { return t->iTunesAccount; }, MP4TagsSetITunesAccount, "MP4TagsSetITunesAccount");
            bind_tag_string(m_raw, cls, "xid", [](const MP4Tags *t)
                            { return t->xid; }, MP4TagsSetXID, "MP4TagsSetXID");

            bind_tag_scalar<uint16_t>(m_raw, cls, "genreType", [](const MP4Tags *t)
                                      { return t->genreType; }, MP4TagsSetGenreType, "MP4TagsSetGenreType");
            bind_tag_scalar<uint8_t>(m_raw, cls, "compilation", [](const MP4Tags *t)
                                     { return t->compilation; }, MP4TagsSetCompilation, "MP4TagsSetCompilation");
            bind_tag_scalar<uint32_t>(m_raw, cls, "tvSeason", [](const MP4Tags *t)
                                      { return t->tvSeason; }, MP4TagsSetTVSeason, "MP4TagsSetTVSeason");
            bind_tag_scalar<uint32_t>(m_raw, cls, "tvEpisode", [](const MP4Tags *t)
                                      { return t->tvEpisode; }, MP4TagsSetTVEpisode, "MP4TagsSetTVEpisode");
            bind_tag_scalar<uint8_t>(m_raw, cls, "podcast", [](const MP4Tags *t)
                                     { return t->podcast; }, MP4TagsSetPodcast, "MP4TagsSetPodcast");
            bind_tag_scalar<uint8_t>(m_raw, cls, "hdVideo", [](const MP4Tags *t)
                                     { return t->hdVideo; }, MP4TagsSetHDVideo, "MP4TagsSetHDVideo");
            bind_tag_scalar<uint8_t>(m_raw, cls, "mediaType", [](const MP4Tags *t)
                                     { return t->mediaType; }, MP4TagsSetMediaType, "MP4TagsSetMediaType");
            bind_tag_scalar<uint8_t>(m_raw, cls, "contentRating", [](const MP4Tags *t)
                                     { return t->contentRating; }, MP4TagsSetContentRating, "MP4TagsSetContentRating");
            bind_tag_scalar<uint8_t>(m_raw, cls, "gapless", [](const MP4Tags *t)
                                     { return t->gapless; }, MP4TagsSetGapless, "MP4TagsSetGapless");
            bind_tag_scalar<uint8_t>(m_raw, cls, "iTunesAccountType", [](const MP4Tags *t)
                                     { return t->iTunesAccountType; }, MP4TagsSetITunesAccountType,
                                     "MP4TagsSetITunesAccountType");
            bind_tag_scalar<uint32_t>(m_raw, cls, "iTunesCountry", [](const MP4Tags *t)
                                      { return t->iTunesCountry; }, MP4TagsSetITunesCountry, "MP4TagsSetITunesCountry");
            bind_tag_scalar<uint32_t>(m_raw, cls, "contentID", [](const MP4Tags *t)
                                      { return t->contentID; }, MP4TagsSetContentID, "MP4TagsSetContentID");
            bind_tag_scalar<uint32_t>(m_raw, cls, "artistID", [](const MP4Tags *t)
                                      { return t->artistID; }, MP4TagsSetArtistID, "MP4TagsSetArtistID");
            bind_tag_scalar<uint64_t>(m_raw, cls, "playlistID", [](const MP4Tags *t)
                                      { return t->playlistID; }, MP4TagsSetPlaylistID, "MP4TagsSetPlaylistID");
            bind_tag_scalar<uint32_t>(m_raw, cls, "genreID", [](const MP4Tags *t)
                                      { return t->genreID; }, MP4TagsSetGenreID, "MP4TagsSetGenreID");
            bind_tag_scalar<uint32_t>(m_raw, cls, "composerID", [](const MP4Tags *t)
                                      { return t->composerID; }, MP4TagsSetComposerID, "MP4TagsSetComposerID");

            m_raw.def("MP4TagsAlloc", []()
                      { return MP4TagsWrapper(); }, py::return_value_policy::move);
            m_raw.def("MP4TagsFree", [](MP4TagsWrapper &tags)
                      { tags.free(); }, py::arg("tags"));
            m_raw.def("MP4TagsFetch", &MP4TagsFetch_wrapper, py::arg("tags"), py::arg("hFile"));
            m_raw.def("MP4TagsStore", &MP4TagsStore_wrapper, py::arg("tags"), py::arg("hFile"));
            m_raw.def("MP4TagsHasMetadata", &MP4TagsHasMetadata_wrapper, py::arg("tags"));
            m_raw.def("MP4TagsAddArtwork", &MP4TagsAddArtwork_wrapper, py::arg("tags"), py::arg("data"),
                      py::arg("type") = MP4_ART_UNDEFINED);
            m_raw.def("MP4TagsSetArtwork", &MP4TagsSetArtwork_wrapper, py::arg("tags"), py::arg("index"),
                      py::arg("data"), py::arg("type") = MP4_ART_UNDEFINED);
            m_raw.def("MP4TagsRemoveArtwork", &MP4TagsRemoveArtwork_wrapper, py::arg("tags"), py::arg("index"));
            m_raw.def(
                "MP4TagsSetTrack",
                [](MP4TagsWrapper &tags, const std::optional<MP4TagTrack> &value)
                {
                    const MP4TagTrack *p = value ? &*value : nullptr;
                    if (!MP4TagsSetTrack(tags.get(), p))
                    {
                        throw MP4Error("MP4TagsSetTrack failed");
                    }
                },
                py::arg("tags"), py::arg("value") = py::none());
            m_raw.def(
                "MP4TagsSetDisk",
                [](MP4TagsWrapper &tags, const std::optional<MP4TagDisk> &value)
                {
                    const MP4TagDisk *p = value ? &*value : nullptr;
                    if (!MP4TagsSetDisk(tags.get(), p))
                    {
                        throw MP4Error("MP4TagsSetDisk failed");
                    }
                },
                py::arg("tags"), py::arg("value") = py::none());
            m_raw.def(
                "MP4TagsSetTempo",
                [](MP4TagsWrapper &tags, const std::optional<uint16_t> &value)
                {
                    const uint16_t *p = value ? &*value : nullptr;
                    if (!MP4TagsSetTempo(tags.get(), p))
                    {
                        throw MP4Error("MP4TagsSetTempo failed");
                    }
                },
                py::arg("tags"), py::arg("value") = py::none());
        }

        void bind_chapters(py::module_ &m_raw)
        {
            m_raw.attr("MP4V2_CHAPTER_TITLE_MAX") = py::int_(MP4V2_CHAPTER_TITLE_MAX);

            py::enum_<MP4ChapterType>(m_raw, "MP4ChapterType")
                .value("MP4ChapterTypeNone", MP4ChapterTypeNone)
                .value("MP4ChapterTypeAny", MP4ChapterTypeAny)
                .value("MP4ChapterTypeQt", MP4ChapterTypeQt)
                .value("MP4ChapterTypeNero", MP4ChapterTypeNero)
                .export_values();

            py::class_<MP4ChapterData>(m_raw, "MP4Chapter",
                                       "One chapter: duration is milliseconds for Get/SetChapters.")
                .def(py::init<>())
                .def(py::init<MP4Duration, std::string>(), py::arg("duration"), py::arg("title") = "")
                .def_readwrite("duration", &MP4ChapterData::duration)
                .def_readwrite("title", &MP4ChapterData::title)
                .def("__repr__",
                     [](const MP4ChapterData &c)
                     {
                         return "<MP4Chapter duration=" + std::to_string(c.duration) + " title=" + c.title + ">";
                     });

            py::class_<MP4ChapterListResult>(m_raw, "MP4ChapterList",
                                             "Result of MP4GetChapters: type plus a list of MP4Chapter.")
                .def_readonly("type", &MP4ChapterListResult::type)
                .def_readonly("chapters", &MP4ChapterListResult::chapters)
                .def("__iter__",
                     [](const MP4ChapterListResult &r)
                     { return py::iter(py::make_tuple(r.type, r.chapters)); })
                .def("__len__", [](const MP4ChapterListResult &r)
                     { return r.chapters.size(); });

            m_raw.def("MP4AddChapterTextTrack", &MP4AddChapterTextTrack_wrapper, py::arg("hFile"),
                      py::arg("refTrackId"), py::arg("timescale") = 0,
                      R"doc(
    Add a QuickTime chapter (text) track. timescale 0 copies the reference track.
)doc");
            m_raw.def("MP4AddChapter", &MP4AddChapter_wrapper, py::arg("hFile"), py::arg("chapterTrackId"),
                      py::arg("chapterDuration"), py::arg("chapterTitle") = py::none(),
                      R"doc(
    Append a QuickTime chapter. Duration is in the chapter track timescale.
    chapterTitle None uses the default "Chapter NNN".
)doc");
            m_raw.def("MP4AddNeroChapter", &MP4AddNeroChapter_wrapper, py::arg("hFile"), py::arg("chapterStart"),
                      py::arg("chapterTitle") = py::none(),
                      R"doc(
    Add a Nero chapter. chapterStart is in 100-nanosecond units.
)doc");
            m_raw.def("MP4ConvertChapters", &MP4ConvertChapters_wrapper, py::arg("hFile"),
                      py::arg("toChapterType") = MP4ChapterTypeQt,
                      "Convert Qt↔Nero. Returns the type before conversion, or MP4ChapterTypeNone.");
            m_raw.def("MP4DeleteChapters", &MP4DeleteChapters_wrapper, py::arg("hFile"),
                      py::arg("chapterType") = MP4ChapterTypeQt,
                      py::arg("chapterTrackId") = MP4_INVALID_TRACK_ID,
                      "Delete chapters of the given type. Returns the type deleted, or MP4ChapterTypeNone.");
            m_raw.def("MP4GetChapters", &MP4GetChapters_wrapper, py::arg("hFile"),
                      py::arg("chapterType") = MP4ChapterTypeQt,
                      R"doc(
    Return MP4ChapterList (unpacks as ``(type, chapters)``).
    Copies the C array and MP4Free's it. Empty if none match; does not raise.
)doc");
            m_raw.def("MP4SetChapters", &MP4SetChapters_wrapper, py::arg("hFile"), py::arg("chapterList"),
                      py::arg("chapterType") = MP4ChapterTypeQt,
                      R"doc(
    Replace chapters of the given type. chapterList is a sequence of MP4Chapter.
    Duration is milliseconds. Raises MP4Error if a non-empty list cannot be written.
)doc");
        }
    } // namespace

    void bind_extended(py::module_ &m_raw)
    {
        bind_constants(m_raw);
        bind_tracks(m_raw);
        bind_props(m_raw);
        bind_samples(m_raw);
        bind_tags(m_raw);
        bind_chapters(m_raw);
    }
} // namespace raw
