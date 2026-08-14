#include "pymp4v2/mp4file.h"

#include <pybind11/stl.h>

#include "mp4v2/mp4v2.h"
#include "pymp4v2/error.h"

namespace py = pybind11;

namespace
{
    std::optional<std::string> tag_str(const char *value)
    {
        if (value == nullptr)
        {
            return std::nullopt;
        }
        return std::string(value);
    }

    py::ssize_t normalize_index(py::ssize_t index, py::ssize_t count)
    {
        if (index < 0)
        {
            index += count;
        }
        if (index < 0 || index >= count)
        {
            throw py::index_error();
        }
        return index;
    }
} // namespace

Track::Track(MP4File *file, MP4TrackId id) : file_(file), id_(id) {}

MP4TrackId Track::id() const
{
    return id_;
}

std::string Track::type() const
{
    return raw::MP4GetTrackType_wrapper(file_->handle(), id_);
}

MP4Duration Track::duration() const
{
    return raw::MP4GetTrackDuration_wrapper(file_->handle(), id_);
}

uint32_t Track::timescale() const
{
    return raw::MP4GetTrackTimeScale_wrapper(file_->handle(), id_);
}

MP4SampleId Track::sample_count() const
{
    return raw::MP4GetTrackNumberOfSamples_wrapper(file_->handle(), id_);
}

std::optional<std::string> Track::language() const
{
    try
    {
        return raw::MP4GetTrackLanguage_wrapper(file_->handle(), id_);
    }
    catch (const MP4Error &)
    {
        return std::nullopt;
    }
}

std::optional<std::string> Track::name() const
{
    try
    {
        return raw::MP4GetTrackName_wrapper(file_->handle(), id_);
    }
    catch (const MP4Error &)
    {
        return std::nullopt;
    }
}

uint16_t Track::width() const
{
    return raw::MP4GetTrackVideoWidth_wrapper(file_->handle(), id_);
}

uint16_t Track::height() const
{
    return raw::MP4GetTrackVideoHeight_wrapper(file_->handle(), id_);
}

uint32_t Track::bitrate() const
{
    return raw::MP4GetTrackBitRate_wrapper(file_->handle(), id_);
}

double Track::frame_rate() const
{
    return raw::MP4GetTrackVideoFrameRate_wrapper(file_->handle(), id_);
}

std::optional<int> Track::audio_channels() const
{
    try
    {
        return raw::MP4GetTrackAudioChannels_wrapper(file_->handle(), id_);
    }
    catch (const MP4Error &)
    {
        return std::nullopt;
    }
}

raw::MP4SampleData Track::read_sample(py::ssize_t index) const
{
    const auto count = static_cast<py::ssize_t>(sample_count());
    const auto i = normalize_index(index, count);
    return raw::MP4ReadSample_wrapper(file_->handle(), id_, static_cast<MP4SampleId>(i + 1));
}

Tracks::Tracks(MP4File *file) : file_(file) {}

py::ssize_t Tracks::size() const
{
    return static_cast<py::ssize_t>(file_->get_track_count());
}

Track Tracks::getitem(py::ssize_t index) const
{
    const auto i = normalize_index(index, size());
    const MP4TrackId id = raw::MP4FindTrackId_wrapper(file_->handle(), static_cast<uint16_t>(i), std::nullopt, 0);
    return Track(file_, id);
}

Tags::Tags(MP4File *file) : file_(file)
{
    fetch();
}

void Tags::fetch()
{
    raw::MP4TagsFetch_wrapper(tags_, file_->handle());
    dirty_ = false;
}

void Tags::store()
{
    check_writable();
    raw::MP4TagsStore_wrapper(tags_, file_->handle());
    dirty_ = false;
}

void Tags::add_artwork(const std::string &data, MP4TagArtworkType type)
{
    check_writable();
    raw::MP4TagsAddArtwork_wrapper(tags_, data, type);
    dirty_ = true;
}

void Tags::remove_artwork(uint32_t index)
{
    check_writable();
    raw::MP4TagsRemoveArtwork_wrapper(tags_, index);
    dirty_ = true;
}

py::list Tags::artwork() const
{
    const MP4Tags *t = tags_.get();
    py::list out;
    for (uint32_t i = 0; i < t->artworkCount; ++i)
    {
        const MP4TagArtwork &a = t->artwork[i];
        if (a.data != nullptr && a.size > 0)
        {
            out.append(py::bytes(static_cast<const char *>(a.data), a.size));
        }
        else
        {
            out.append(py::bytes());
        }
    }
    return out;
}

uint32_t Tags::artwork_count() const
{
    return tags_.get()->artworkCount;
}

bool Tags::dirty() const
{
    return dirty_;
}

void Tags::mark_dirty()
{
    dirty_ = true;
}

void Tags::set_file(MP4File *file)
{
    file_ = file;
}

void Tags::check_writable() const
{
    if (file_->mode() == "r")
    {
        throw MP4Error("Cannot modify tags on a read-only MP4File: " + file_->filename());
    }
}

raw::MP4TagsWrapper &Tags::raw_tags()
{
    return tags_;
}

const raw::MP4TagsWrapper &Tags::raw_tags() const
{
    return tags_;
}

MP4File::MP4File(const std::string &filename, const std::string &mode)
    : filename_(filename), mode_(mode)
{
    open();
}

MP4File::MP4File(MP4File &&other) noexcept
    : filename_(std::move(other.filename_)), mode_(std::move(other.mode_)), handle_(std::move(other.handle_)),
      tags_(std::move(other.tags_))
{
    if (tags_)
    {
        tags_->set_file(this);
    }
}

MP4File &MP4File::operator=(MP4File &&other) noexcept
{
    if (this != &other)
    {
        filename_ = std::move(other.filename_);
        mode_ = std::move(other.mode_);
        handle_ = std::move(other.handle_);
        tags_ = std::move(other.tags_);
        if (tags_)
        {
            tags_->set_file(this);
        }
    }
    return *this;
}

void MP4File::open()
{
    MP4FileHandle h = nullptr;
    std::string action;

    if (mode_ != "r" && mode_ != "w" && mode_ != "a" && mode_ != "r+")
    {
        throw MP4Error("Unsupported mode '" + mode_ + "' for file: " + filename_);
    }

    {
        py::gil_scoped_release release;
        if (mode_ == "r")
        {
            action = "open";
            h = MP4Read(filename_.c_str());
        }
        else if (mode_ == "w")
        {
            action = "create";
            h = MP4Create(filename_.c_str());
        }
        else
        {
            action = "modify";
            h = MP4Modify(filename_.c_str());
        }
    }

    if (h == nullptr)
    {
        throw MP4Error("Failed to " + action + " MP4 file: " + filename_);
    }

    handle_ = raw::MP4FileHandleWrapper(h);
}

void MP4File::ensure_open() const
{
    if (!handle_.is_valid())
    {
        throw MP4Error("MP4 file is closed: " + filename_);
    }
}

raw::MP4FileHandleWrapper &MP4File::handle()
{
    ensure_open();
    return handle_;
}

void MP4File::reopen_existing()
{
    MP4FileHandle h = nullptr;
    {
        py::gil_scoped_release release;
        if (mode_ == "r")
        {
            h = MP4Read(filename_.c_str());
        }
        else
        {
            h = MP4Modify(filename_.c_str());
        }
    }
    if (h == nullptr)
    {
        throw MP4Error("Failed to reopen MP4 file: " + filename_);
    }
    handle_ = raw::MP4FileHandleWrapper(h);
    if (tags_)
    {
        tags_->fetch();
    }
}

void MP4File::close()
{
    if (tags_ && tags_->dirty() && handle_.is_valid() && mode_ != "r")
    {
        tags_->store();
    }
    handle_.close();
}

int MP4File::get_track_count() const
{
    ensure_open();
    return static_cast<int>(MP4GetNumberOfTracks(handle_.get()));
}

void MP4File::save()
{
    ensure_open();
    if (mode_ == "r")
    {
        return;
    }

    close();
    reopen_existing();
}

bool MP4File::is_open() const
{
    return handle_.is_valid();
}

std::string MP4File::get_info() const
{
    ensure_open();
    const char *info = MP4Info(handle_.get());
    if (info == nullptr)
    {
        return "";
    }
    std::string result(info);
    MP4Free(const_cast<char *>(info));
    return result;
}

const std::string &MP4File::filename() const
{
    return filename_;
}

const std::string &MP4File::mode() const
{
    return mode_;
}

MP4Duration MP4File::duration() const
{
    ensure_open();
    return MP4GetDuration(handle_.get());
}

uint32_t MP4File::timescale() const
{
    ensure_open();
    return MP4GetTimeScale(handle_.get());
}

void MP4File::optimize(const std::optional<std::string> &newFileName)
{
    const bool reopen = is_open();
    if (reopen)
    {
        close();
    }
    raw::MP4Optimize_wrapper(filename_.c_str(), newFileName ? newFileName->c_str() : nullptr);
    if (reopen)
    {
        reopen_existing();
    }
}

Tracks MP4File::tracks()
{
    ensure_open();
    return Tracks(this);
}

Tags &MP4File::tags()
{
    ensure_open();
    if (!tags_)
    {
        tags_ = std::make_unique<Tags>(this);
    }
    return *tags_;
}

void bind_highlevel(py::module_ &m, py::module_ &m_raw)
{
    py::class_<Track>(m, "Track", "One track of an MP4File. Sample indices are 0-based.")
        .def_property_readonly("id", &Track::id)
        .def_property_readonly("type", &Track::type)
        .def_property_readonly("duration", &Track::duration)
        .def_property_readonly("timescale", &Track::timescale)
        .def_property_readonly("sample_count", &Track::sample_count)
        .def_property_readonly("language", &Track::language)
        .def_property_readonly("name", &Track::name)
        .def_property_readonly("width", &Track::width)
        .def_property_readonly("height", &Track::height)
        .def_property_readonly("bitrate", &Track::bitrate)
        .def_property_readonly("frame_rate", &Track::frame_rate)
        .def_property_readonly("audio_channels", &Track::audio_channels)
        .def("read_sample", &Track::read_sample, py::arg("index"),
             "Read sample at 0-based index. Returns Sample (data + timing).")
        .def("__len__", &Track::sample_count)
        .def("__getitem__", &Track::read_sample, py::arg("index"))
        .def("__repr__",
             [](const Track &t)
             {
                 return "<Track id=" + std::to_string(t.id()) + " type=" + t.type() + ">";
             });

    py::class_<Tracks>(m, "Tracks", "Sequence of Track objects on an MP4File.")
        .def("__len__", &Tracks::size)
        .def("__getitem__", &Tracks::getitem, py::arg("index"), py::keep_alive<0, 1>());

    auto tags_cls = py::class_<Tags>(m, "Tags",
                                     "iTMF tags on an MP4File. Fetch on first access; store() on "
                                     "context exit if dirty. Artwork is a list of bytes.");
    tags_cls.def("fetch", &Tags::fetch)
        .def("store", &Tags::store)
        .def("add_artwork", &Tags::add_artwork, py::arg("data"), py::arg("type") = MP4_ART_UNDEFINED)
        .def("remove_artwork", &Tags::remove_artwork, py::arg("index"))
        .def_property_readonly("artwork", &Tags::artwork)
        .def_property_readonly("artwork_count", &Tags::artwork_count)
        .def("__enter__", [](Tags &self) -> Tags &
             { return self; })
        .def("__exit__",
             [](Tags &self, const py::object &, const py::object &, const py::object &)
             {
                 if (self.dirty())
                 {
                     self.store();
                 }
                 return false;
             });

    auto bind_tag = [&](const char *pyname, const char *(*getter)(const MP4Tags *),
                        bool (*setter)(const MP4Tags *, const char *), const char *setter_name)
    {
        tags_cls.def_property(
            pyname,
            [getter](const Tags &self)
            { return tag_str(getter(self.raw_tags().get())); },
            [setter, setter_name](Tags &self, const std::optional<std::string> &value)
            {
                self.check_writable();
                raw::MP4TagsSetString_wrapper(self.raw_tags(), setter, value, setter_name);
                self.mark_dirty();
            });
    };

    bind_tag("name", [](const MP4Tags *t)
             { return t->name; }, MP4TagsSetName, "MP4TagsSetName");
    bind_tag("artist", [](const MP4Tags *t)
             { return t->artist; }, MP4TagsSetArtist, "MP4TagsSetArtist");
    bind_tag("album_artist", [](const MP4Tags *t)
             { return t->albumArtist; }, MP4TagsSetAlbumArtist, "MP4TagsSetAlbumArtist");
    bind_tag("album", [](const MP4Tags *t)
             { return t->album; }, MP4TagsSetAlbum, "MP4TagsSetAlbum");
    bind_tag("grouping", [](const MP4Tags *t)
             { return t->grouping; }, MP4TagsSetGrouping, "MP4TagsSetGrouping");
    bind_tag("composer", [](const MP4Tags *t)
             { return t->composer; }, MP4TagsSetComposer, "MP4TagsSetComposer");
    bind_tag("comments", [](const MP4Tags *t)
             { return t->comments; }, MP4TagsSetComments, "MP4TagsSetComments");
    bind_tag("genre", [](const MP4Tags *t)
             { return t->genre; }, MP4TagsSetGenre, "MP4TagsSetGenre");
    bind_tag("release_date", [](const MP4Tags *t)
             { return t->releaseDate; }, MP4TagsSetReleaseDate, "MP4TagsSetReleaseDate");
    bind_tag("description", [](const MP4Tags *t)
             { return t->description; }, MP4TagsSetDescription, "MP4TagsSetDescription");
    bind_tag("long_description", [](const MP4Tags *t)
             { return t->longDescription; }, MP4TagsSetLongDescription, "MP4TagsSetLongDescription");
    bind_tag("lyrics", [](const MP4Tags *t)
             { return t->lyrics; }, MP4TagsSetLyrics, "MP4TagsSetLyrics");
    bind_tag("copyright", [](const MP4Tags *t)
             { return t->copyright; }, MP4TagsSetCopyright, "MP4TagsSetCopyright");
    bind_tag("encoding_tool", [](const MP4Tags *t)
             { return t->encodingTool; }, MP4TagsSetEncodingTool, "MP4TagsSetEncodingTool");
    bind_tag("encoded_by", [](const MP4Tags *t)
             { return t->encodedBy; }, MP4TagsSetEncodedBy, "MP4TagsSetEncodedBy");
    bind_tag("tv_show", [](const MP4Tags *t)
             { return t->tvShow; }, MP4TagsSetTVShow, "MP4TagsSetTVShow");
    bind_tag("tv_network", [](const MP4Tags *t)
             { return t->tvNetwork; }, MP4TagsSetTVNetwork, "MP4TagsSetTVNetwork");
    bind_tag("tv_episode_id", [](const MP4Tags *t)
             { return t->tvEpisodeID; }, MP4TagsSetTVEpisodeID, "MP4TagsSetTVEpisodeID");
    bind_tag("sort_name", [](const MP4Tags *t)
             { return t->sortName; }, MP4TagsSetSortName, "MP4TagsSetSortName");
    bind_tag("sort_artist", [](const MP4Tags *t)
             { return t->sortArtist; }, MP4TagsSetSortArtist, "MP4TagsSetSortArtist");
    bind_tag("sort_album_artist", [](const MP4Tags *t)
             { return t->sortAlbumArtist; }, MP4TagsSetSortAlbumArtist, "MP4TagsSetSortAlbumArtist");
    bind_tag("sort_album", [](const MP4Tags *t)
             { return t->sortAlbum; }, MP4TagsSetSortAlbum, "MP4TagsSetSortAlbum");
    bind_tag("sort_composer", [](const MP4Tags *t)
             { return t->sortComposer; }, MP4TagsSetSortComposer, "MP4TagsSetSortComposer");
    bind_tag("sort_tv_show", [](const MP4Tags *t)
             { return t->sortTVShow; }, MP4TagsSetSortTVShow, "MP4TagsSetSortTVShow");
    bind_tag("purchase_date", [](const MP4Tags *t)
             { return t->purchaseDate; }, MP4TagsSetPurchaseDate, "MP4TagsSetPurchaseDate");
    bind_tag("keywords", [](const MP4Tags *t)
             { return t->keywords; }, MP4TagsSetKeywords, "MP4TagsSetKeywords");
    bind_tag("category", [](const MP4Tags *t)
             { return t->category; }, MP4TagsSetCategory, "MP4TagsSetCategory");
    bind_tag("itunes_account", [](const MP4Tags *t)
             { return t->iTunesAccount; }, MP4TagsSetITunesAccount, "MP4TagsSetITunesAccount");
    bind_tag("xid", [](const MP4Tags *t)
             { return t->xid; }, MP4TagsSetXID, "MP4TagsSetXID");

    auto bind_u8 = [&](const char *pyname, const uint8_t *(*getter)(const MP4Tags *),
                       bool (*setter)(const MP4Tags *, const uint8_t *), const char *setter_name)
    {
        tags_cls.def_property(
            pyname,
            [getter](const Tags &self) -> std::optional<uint8_t>
            {
                const uint8_t *p = getter(self.raw_tags().get());
                if (p == nullptr)
                {
                    return std::nullopt;
                }
                return *p;
            },
            [setter, setter_name](Tags &self, const std::optional<uint8_t> &value)
            {
                self.check_writable();
                raw::MP4TagsSetScalar_wrapper(self.raw_tags(), setter, value, setter_name);
                self.mark_dirty();
            });
    };
    auto bind_u16 = [&](const char *pyname, const uint16_t *(*getter)(const MP4Tags *),
                        bool (*setter)(const MP4Tags *, const uint16_t *), const char *setter_name)
    {
        tags_cls.def_property(
            pyname,
            [getter](const Tags &self) -> std::optional<uint16_t>
            {
                const uint16_t *p = getter(self.raw_tags().get());
                if (p == nullptr)
                {
                    return std::nullopt;
                }
                return *p;
            },
            [setter, setter_name](Tags &self, const std::optional<uint16_t> &value)
            {
                self.check_writable();
                raw::MP4TagsSetScalar_wrapper(self.raw_tags(), setter, value, setter_name);
                self.mark_dirty();
            });
    };
    auto bind_u32 = [&](const char *pyname, const uint32_t *(*getter)(const MP4Tags *),
                        bool (*setter)(const MP4Tags *, const uint32_t *), const char *setter_name)
    {
        tags_cls.def_property(
            pyname,
            [getter](const Tags &self) -> std::optional<uint32_t>
            {
                const uint32_t *p = getter(self.raw_tags().get());
                if (p == nullptr)
                {
                    return std::nullopt;
                }
                return *p;
            },
            [setter, setter_name](Tags &self, const std::optional<uint32_t> &value)
            {
                self.check_writable();
                raw::MP4TagsSetScalar_wrapper(self.raw_tags(), setter, value, setter_name);
                self.mark_dirty();
            });
    };
    auto bind_u64 = [&](const char *pyname, const uint64_t *(*getter)(const MP4Tags *),
                        bool (*setter)(const MP4Tags *, const uint64_t *), const char *setter_name)
    {
        tags_cls.def_property(
            pyname,
            [getter](const Tags &self) -> std::optional<uint64_t>
            {
                const uint64_t *p = getter(self.raw_tags().get());
                if (p == nullptr)
                {
                    return std::nullopt;
                }
                return *p;
            },
            [setter, setter_name](Tags &self, const std::optional<uint64_t> &value)
            {
                self.check_writable();
                raw::MP4TagsSetScalar_wrapper(self.raw_tags(), setter, value, setter_name);
                self.mark_dirty();
            });
    };

    bind_u16("genre_type", [](const MP4Tags *t)
             { return t->genreType; }, MP4TagsSetGenreType, "MP4TagsSetGenreType");
    bind_u16("tempo", [](const MP4Tags *t)
             { return t->tempo; }, MP4TagsSetTempo, "MP4TagsSetTempo");
    bind_u8("compilation", [](const MP4Tags *t)
            { return t->compilation; }, MP4TagsSetCompilation, "MP4TagsSetCompilation");
    bind_u32("tv_season", [](const MP4Tags *t)
             { return t->tvSeason; }, MP4TagsSetTVSeason, "MP4TagsSetTVSeason");
    bind_u32("tv_episode", [](const MP4Tags *t)
             { return t->tvEpisode; }, MP4TagsSetTVEpisode, "MP4TagsSetTVEpisode");
    bind_u8("podcast", [](const MP4Tags *t)
            { return t->podcast; }, MP4TagsSetPodcast, "MP4TagsSetPodcast");
    bind_u8("hd_video", [](const MP4Tags *t)
            { return t->hdVideo; }, MP4TagsSetHDVideo, "MP4TagsSetHDVideo");
    bind_u8("media_type", [](const MP4Tags *t)
            { return t->mediaType; }, MP4TagsSetMediaType, "MP4TagsSetMediaType");
    bind_u8("content_rating", [](const MP4Tags *t)
            { return t->contentRating; }, MP4TagsSetContentRating, "MP4TagsSetContentRating");
    bind_u8("gapless", [](const MP4Tags *t)
            { return t->gapless; }, MP4TagsSetGapless, "MP4TagsSetGapless");
    bind_u8("itunes_account_type", [](const MP4Tags *t)
            { return t->iTunesAccountType; }, MP4TagsSetITunesAccountType, "MP4TagsSetITunesAccountType");
    bind_u32("itunes_country", [](const MP4Tags *t)
             { return t->iTunesCountry; }, MP4TagsSetITunesCountry, "MP4TagsSetITunesCountry");
    bind_u32("content_id", [](const MP4Tags *t)
             { return t->contentID; }, MP4TagsSetContentID, "MP4TagsSetContentID");
    bind_u32("artist_id", [](const MP4Tags *t)
             { return t->artistID; }, MP4TagsSetArtistID, "MP4TagsSetArtistID");
    bind_u64("playlist_id", [](const MP4Tags *t)
             { return t->playlistID; }, MP4TagsSetPlaylistID, "MP4TagsSetPlaylistID");
    bind_u32("genre_id", [](const MP4Tags *t)
             { return t->genreID; }, MP4TagsSetGenreID, "MP4TagsSetGenreID");
    bind_u32("composer_id", [](const MP4Tags *t)
             { return t->composerID; }, MP4TagsSetComposerID, "MP4TagsSetComposerID");

    tags_cls.def_property(
        "track",
        [](const Tags &self) -> std::optional<MP4TagTrack>
        {
            if (self.raw_tags().get()->track == nullptr)
            {
                return std::nullopt;
            }
            return *self.raw_tags().get()->track;
        },
        [](Tags &self, const std::optional<MP4TagTrack> &value)
        {
            self.check_writable();
            const MP4TagTrack *p = value ? &*value : nullptr;
            if (!MP4TagsSetTrack(self.raw_tags().get(), p))
            {
                throw MP4Error("MP4TagsSetTrack failed");
            }
            self.mark_dirty();
        });
    tags_cls.def_property(
        "disk",
        [](const Tags &self) -> std::optional<MP4TagDisk>
        {
            if (self.raw_tags().get()->disk == nullptr)
            {
                return std::nullopt;
            }
            return *self.raw_tags().get()->disk;
        },
        [](Tags &self, const std::optional<MP4TagDisk> &value)
        {
            self.check_writable();
            const MP4TagDisk *p = value ? &*value : nullptr;
            if (!MP4TagsSetDisk(self.raw_tags().get(), p))
            {
                throw MP4Error("MP4TagsSetDisk failed");
            }
            self.mark_dirty();
        });

    py::class_<MP4File>(m, "MP4File",
                        "High-level MP4 file object. Owns a handle via RAII; close() (and "
                        "__exit__) persist pending writes for create/modify handles.")
        .def(py::init<const std::string &, const std::string &>(), py::arg("filename"), py::arg("mode") = "r",
             R"doc(
    Open or create an MP4 file.

    Args:
        filename: Path to the file.
        mode: ``"r"`` (read, default), ``"w"`` (create/truncate), ``"a"`` or ``"r+"`` (modify).

    Raises:
        MP4Error: If the mode is unknown or the file cannot be opened. Errors include the filename.
)doc")
        .def("close", &MP4File::close,
             "Close the file. For create/modify handles this writes pending changes "
             "and stores dirty tags. Safe to call twice.")
        .def("get_track_count", &MP4File::get_track_count,
             "Number of tracks. Raises MP4Error if the file is closed.")
        .def("save", &MP4File::save,
             R"doc(
    Persist pending writes and keep the file open.

    mp4v2 has no flush API: writes happen on close. save() closes then reopens with
    MP4Modify so the object stays usable. Read-only files are a no-op. Never recreates
    (that would truncate). Raises MP4Error if the file is already closed.
)doc")
        .def("is_open", &MP4File::is_open, "True if the handle is still valid.")
        .def("get_info", &MP4File::get_info,
             "Textual summary of the file. Raises MP4Error if the file is closed.")
        .def_property_readonly("info", &MP4File::get_info)
        .def_property_readonly("filename", &MP4File::filename)
        .def_property_readonly("mode", &MP4File::mode)
        .def_property_readonly("duration", &MP4File::duration, "Movie duration in movie timescale units.")
        .def_property_readonly("timescale", &MP4File::timescale, "Movie timescale (ticks per second).")
        .def_property_readonly("tracks", py::cpp_function(&MP4File::tracks, py::keep_alive<0, 1>()))
        .def_property_readonly("tags", &MP4File::tags, py::return_value_policy::reference_internal)
        .def("optimize", &MP4File::optimize, py::arg("new_filename") = py::none(),
             R"doc(
    Rewrite the file with interleaved samples and moov at the front (MP4Optimize).

    Closes the handle first so pending writes are on disk, then reopens if it was open.
    ``new_filename`` None optimizes in place.
)doc")
        .def("__enter__", [](MP4File &self) -> MP4File &
             { return self; }, py::return_value_policy::reference)
        .def("__exit__", [](MP4File &self, const py::object &, const py::object &, const py::object &)
             {
                 self.close();
                 return false; });

    m.attr("Sample") = m_raw.attr("MP4Sample");
}
