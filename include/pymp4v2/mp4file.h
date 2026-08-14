#ifndef PYMP4V2_MP4FILE_H
#define PYMP4V2_MP4FILE_H

#include <memory>
#include <optional>
#include <string>

#include <pybind11/pybind11.h>

namespace py = pybind11;

#include "mp4v2/mp4v2.h"
#include "pymp4v2/mp4_file_handle_wrapper.h"
#include "pymp4v2/mp4_tags_wrapper.h"
#include "pymp4v2/raw.h"

class MP4File;

class Track
{
public:
    Track(MP4File *file, MP4TrackId id);

    MP4TrackId id() const;
    std::string type() const;
    MP4Duration duration() const;
    uint32_t timescale() const;
    MP4SampleId sample_count() const;
    std::optional<std::string> language() const;
    std::optional<std::string> name() const;
    uint16_t width() const;
    uint16_t height() const;
    uint32_t bitrate() const;
    double frame_rate() const;
    std::optional<int> audio_channels() const;
    raw::MP4SampleData read_sample(py::ssize_t index) const;

private:
    MP4File *file_;
    MP4TrackId id_;
};

class Tracks
{
public:
    explicit Tracks(MP4File *file);
    py::ssize_t size() const;
    Track getitem(py::ssize_t index) const;

private:
    MP4File *file_;
};

class Tags
{
public:
    explicit Tags(MP4File *file);

    void fetch();
    void store();
    void add_artwork(const std::string &data, MP4TagArtworkType type);
    void remove_artwork(uint32_t index);
    py::list artwork() const;
    uint32_t artwork_count() const;

    bool dirty() const;
    void mark_dirty();
    void set_file(MP4File *file);
    void check_writable() const;
    raw::MP4TagsWrapper &raw_tags();
    const raw::MP4TagsWrapper &raw_tags() const;

private:
    MP4File *file_;
    raw::MP4TagsWrapper tags_;
    bool dirty_ = false;
};

class MP4File
{
    friend class Track;
    friend class Tracks;
    friend class Tags;

public:
    MP4File(const std::string &filename, const std::string &mode = "r");
    ~MP4File() = default;

    MP4File(const MP4File &) = delete;
    MP4File &operator=(const MP4File &) = delete;
    MP4File(MP4File &&other) noexcept;
    MP4File &operator=(MP4File &&other) noexcept;

    void close();
    int get_track_count() const;
    void save();
    bool is_open() const;
    std::string get_info() const;
    const std::string &filename() const;
    const std::string &mode() const;
    MP4Duration duration() const;
    uint32_t timescale() const;
    void optimize(const std::optional<std::string> &newFileName = std::nullopt);

    Tracks tracks();
    Tags &tags();

private:
    void open();
    void ensure_open() const;
    void reopen_existing();
    raw::MP4FileHandleWrapper &handle();

    std::string filename_;
    std::string mode_;
    raw::MP4FileHandleWrapper handle_;
    std::unique_ptr<Tags> tags_;
};

void bind_highlevel(pybind11::module_ &m, pybind11::module_ &m_raw);

#endif // PYMP4V2_MP4FILE_H
