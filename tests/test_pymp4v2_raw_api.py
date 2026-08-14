import pytest

import pymp4v2
import pymp4v2.raw as raw


def test_mp4error_is_runtimeerror():
    assert issubclass(pymp4v2.MP4Error, RuntimeError)
    assert pymp4v2.raw.MP4Error is pymp4v2.MP4Error


def test_number_of_tracks_and_type(test_mp4_file):
    with raw.MP4Read(test_mp4_file) as handle:
        assert raw.MP4GetNumberOfTracks(handle) == 1
        assert raw.MP4GetNumberOfTracks(handle, raw.MP4_VIDEO_TRACK_TYPE) == 1
        assert raw.MP4GetNumberOfTracks(handle, raw.MP4_AUDIO_TRACK_TYPE) == 0
        track_id = raw.MP4FindTrackId(handle, 0)
        assert track_id != raw.MP4_INVALID_TRACK_ID
        assert raw.MP4GetTrackType(handle, track_id) == raw.MP4_VIDEO_TRACK_TYPE
        with pytest.raises(pymp4v2.MP4Error):
            raw.MP4FindTrackId(handle, 0, raw.MP4_AUDIO_TRACK_TYPE)


def test_delete_track(writable_mp4):
    with raw.MP4Modify(writable_mp4) as handle:
        track_id = raw.MP4FindTrackId(handle, 0)
        raw.MP4DeleteTrack(handle, track_id)
        assert raw.MP4GetNumberOfTracks(handle) == 0


def test_closed_handle_raises_mp4error():
    handle = raw.MP4FileHandle()
    with pytest.raises(pymp4v2.MP4Error):
        raw.MP4GetNumberOfTracks(handle)


def test_file_and_track_props(test_mp4_file):
    with raw.MP4Read(test_mp4_file) as handle:
        assert raw.MP4HaveAtom(handle, "moov")
        assert not raw.MP4HaveAtom(handle, "moov.udta.meta.ilst")
        assert raw.MP4GetTimeScale(handle) == 1000
        assert raw.MP4GetDuration(handle) == 0
        track_id = raw.MP4FindTrackId(handle, 0)
        assert raw.MP4GetTrackTimeScale(handle, track_id) == 1000
        assert raw.MP4GetTrackDuration(handle, track_id) == 0
        lang = raw.MP4GetTrackLanguage(handle, track_id)
        assert isinstance(lang, str)
        assert len(lang) == 3


def test_add_video_track_size_and_language(temp_mp4_file):
    with raw.MP4Create(temp_mp4_file) as handle:
        track_id = raw.MP4AddVideoTrack(handle, 90000, 3000, 320, 240)
        assert raw.MP4GetTrackVideoWidth(handle, track_id) == 320
        assert raw.MP4GetTrackVideoHeight(handle, track_id) == 240
        raw.MP4SetTrackLanguage(handle, track_id, "eng")
        assert raw.MP4GetTrackLanguage(handle, track_id) == "eng"
        raw.MP4SetTrackName(handle, track_id, "camera")
        assert raw.MP4GetTrackName(handle, track_id) == "camera"
        raw.MP4SetTimeScale(handle, 1000)
        assert raw.MP4GetTimeScale(handle) == 1000


def test_sample_roundtrip(temp_mp4_file):
    payload = b"\x00\x01\x02\xff\x00"
    with raw.MP4Create(temp_mp4_file) as handle:
        track_id = raw.MP4AddAudioTrack(handle, 44100, 1024)
        raw.MP4WriteSample(handle, track_id, payload, duration=1024)
        assert raw.MP4GetTrackNumberOfSamples(handle, track_id) == 1
        assert raw.MP4GetSampleSize(handle, track_id, 1) == len(payload)
        assert raw.MP4GetTrackMaxSampleSize(handle, track_id) == len(payload)

    with raw.MP4Read(temp_mp4_file) as handle:
        track_id = raw.MP4FindTrackId(handle, 0, raw.MP4_AUDIO_TRACK_TYPE)
        sample = raw.MP4ReadSample(handle, track_id, 1)
        assert sample.data == payload
        assert sample.duration == 1024
        data, start, duration, offset, sync = sample
        assert data == payload
        assert duration == 1024
        assert isinstance(start, int)
        assert isinstance(offset, int)
        assert isinstance(sync, bool)
        with pytest.raises(pymp4v2.MP4Error):
            raw.MP4ReadSample(handle, track_id, 99)


def test_sample_time_and_copy_roundtrip(temp_mp4_file, tmp_path):
    payloads = [b"\x00\x01", b"\xfe\xff", b"\xaa\xbb"]
    durations = [1000, 2000, 1500]
    with raw.MP4Create(temp_mp4_file) as handle:
        track_id = raw.MP4AddAudioTrack(handle, 1000, raw.MP4_INVALID_DURATION)
        raw.MP4WriteSample(handle, track_id, payloads[0], duration=durations[0], isSyncSample=True)
        raw.MP4WriteSample(handle, track_id, payloads[1], duration=durations[1], isSyncSample=False)
        raw.MP4WriteSample(handle, track_id, payloads[2], duration=durations[2], isSyncSample=True)

    with raw.MP4Read(temp_mp4_file) as handle:
        track_id = raw.MP4FindTrackId(handle, 0, raw.MP4_AUDIO_TRACK_TYPE)
        assert raw.MP4GetSampleTime(handle, track_id, 1) == 0
        assert raw.MP4GetSampleDuration(handle, track_id, 1) == 1000
        assert raw.MP4GetSampleTime(handle, track_id, 2) == 1000
        assert raw.MP4GetSampleDuration(handle, track_id, 2) == 2000
        assert raw.MP4GetSampleTime(handle, track_id, 3) == 3000
        assert raw.MP4GetSampleSync(handle, track_id, 1) is True
        assert raw.MP4GetSampleSync(handle, track_id, 2) is False
        assert raw.MP4GetSampleIdFromTime(handle, track_id, 0) == 1
        assert raw.MP4GetSampleIdFromTime(handle, track_id, 1000) == 2
        assert raw.MP4GetSampleIdFromTime(handle, track_id, 2500) == 2
        assert raw.MP4GetSampleIdFromTime(handle, track_id, 3000) == 3
        from_time = raw.MP4ReadSampleFromTime(handle, track_id, 1000)
        by_id = raw.MP4ReadSample(handle, track_id, 2)
        assert from_time.data == payloads[1] == by_id.data
        assert from_time.duration == 2000
        assert from_time.isSyncSample is False
        with pytest.raises(pymp4v2.MP4Error):
            raw.MP4GetSampleTime(handle, track_id, 99)
        with pytest.raises(pymp4v2.MP4Error):
            raw.MP4GetSampleDuration(handle, track_id, 99)
        assert raw.MP4GetSampleSync(handle, track_id, 99) is False
        with pytest.raises(pymp4v2.MP4Error):
            raw.MP4GetSampleIdFromTime(handle, 99, 0)
        with pytest.raises(pymp4v2.MP4Error):
            raw.MP4ReadSampleFromTime(handle, 99, 0)

    dest = str(tmp_path / "copied.mp4")
    with raw.MP4Read(temp_mp4_file) as src, raw.MP4Create(dest) as dst:
        src_id = raw.MP4FindTrackId(src, 0, raw.MP4_AUDIO_TRACK_TYPE)
        dst_id = raw.MP4AddAudioTrack(dst, 1000, raw.MP4_INVALID_DURATION)
        raw.MP4CopySample(src, src_id, 2, dst, dst_id)
        raw.MP4CopySample(src, src_id, 1, dst, dst_id, 500)

    with raw.MP4Read(dest) as handle:
        track_id = raw.MP4FindTrackId(handle, 0, raw.MP4_AUDIO_TRACK_TYPE)
        assert raw.MP4GetTrackNumberOfSamples(handle, track_id) == 2
        assert raw.MP4ReadSample(handle, track_id, 1).data == payloads[1]
        copied = raw.MP4ReadSample(handle, track_id, 2)
        assert copied.data == payloads[0]
        assert copied.duration == 500


def test_copy_sample_same_file(temp_mp4_file):
    payload = b"\x11\x22\x33"
    with raw.MP4Create(temp_mp4_file) as handle:
        track_id = raw.MP4AddAudioTrack(handle, 44100, 1024)
        raw.MP4WriteSample(handle, track_id, payload, duration=1024)

    with raw.MP4Modify(temp_mp4_file) as handle:
        track_id = raw.MP4FindTrackId(handle, 0, raw.MP4_AUDIO_TRACK_TYPE)
        raw.MP4CopySample(handle, track_id, 1)
        assert raw.MP4GetTrackNumberOfSamples(handle, track_id) == 2
        other = raw.MP4AddAudioTrack(handle, 44100, 1024)
        raw.MP4CopySample(handle, track_id, 1, handle, other)
        assert raw.MP4GetTrackNumberOfSamples(handle, other) == 1

    with raw.MP4Read(temp_mp4_file) as handle:
        first = raw.MP4FindTrackId(handle, 0, raw.MP4_AUDIO_TRACK_TYPE)
        second = raw.MP4FindTrackId(handle, 1, raw.MP4_AUDIO_TRACK_TYPE)
        assert raw.MP4ReadSample(handle, first, 1).data == payload
        assert raw.MP4ReadSample(handle, first, 2).data == payload
        assert raw.MP4ReadSample(handle, second, 1).data == payload


def test_file_and_track_generic_properties(temp_mp4_file):
    with raw.MP4Create(temp_mp4_file) as handle:
        track_id = raw.MP4AddVideoTrack(handle, 90000, 3000, 320, 240)
        raw.MP4WriteSample(handle, track_id, b"\x00\x01", duration=3000)
        raw.MP4WriteSample(handle, track_id, b"\x02\x03", duration=3000)
        assert raw.MP4GetIntegerProperty(handle, "moov.mvhd.timescale") == raw.MP4GetTimeScale(handle)
        brand = raw.MP4GetStringProperty(handle, "ftyp.majorBrand")
        assert isinstance(brand, str)
        assert len(brand) >= 4
        rate = raw.MP4GetFloatProperty(handle, "moov.mvhd.rate")
        assert rate > 0
        raw.MP4SetFloatProperty(handle, "moov.mvhd.rate", 1.0)
        assert raw.MP4GetFloatProperty(handle, "moov.mvhd.rate") == pytest.approx(1.0)
        assert raw.MP4GetTrackFixedSampleDuration(handle, track_id) == 3000
        assert raw.MP4GetTrackVideoFrameRate(handle, track_id) == pytest.approx(30.0, rel=0.05)
        raw.MP4SetIntegerProperty(handle, "moov.mvhd.timescale", 2000)
        assert raw.MP4GetTimeScale(handle) == 2000
        raw.MP4ChangeMovieTimeScale(handle, 1000)
        assert raw.MP4GetTimeScale(handle) == 1000
        assert raw.MP4HaveTrackAtom(handle, track_id, "tkhd")
        assert not raw.MP4HaveTrackAtom(handle, track_id, "nope")
        assert raw.MP4GetTrackMediaDataName(handle, track_id)
        layer = raw.MP4GetTrackIntegerProperty(handle, track_id, "tkhd.layer")
        assert isinstance(layer, int)
        raw.MP4SetTrackIntegerProperty(handle, track_id, "tkhd.layer", 3)
        assert raw.MP4GetTrackIntegerProperty(handle, track_id, "tkhd.layer") == 3
        volume = raw.MP4GetTrackFloatProperty(handle, track_id, "tkhd.volume")
        assert isinstance(volume, float)
        tk_matrix = raw.MP4GetTrackBytesProperty(handle, track_id, "tkhd.matrix")
        assert isinstance(tk_matrix, bytes)
        cfg = b"\x01\x02\x03\x04"
        raw.MP4SetTrackESConfiguration(handle, track_id, cfg)
        assert raw.MP4GetTrackESConfiguration(handle, track_id) == cfg
        with pytest.raises(pymp4v2.MP4Error):
            raw.MP4GetIntegerProperty(handle, "no.such.property")
        with pytest.raises(pymp4v2.MP4Error):
            raw.MP4GetBytesProperty(handle, "no.such.property")
        with pytest.raises(pymp4v2.MP4Error):
            raw.MP4GetTrackAudioChannels(handle, track_id)

    with raw.MP4Read(temp_mp4_file) as handle:
        track_id = raw.MP4FindTrackId(handle, 0)
        bitrate = raw.MP4GetTrackBitRate(handle, track_id)
        assert bitrate >= 0


def test_audio_channels_and_es_config(temp_mp4_file):
    with raw.MP4Create(temp_mp4_file) as handle:
        track_id = raw.MP4AddAudioTrack(handle, 44100, 1024)
        raw.MP4WriteSample(handle, track_id, b"\x00\x01\x02", duration=1024)
        channels = raw.MP4GetTrackAudioChannels(handle, track_id)
        assert channels >= 0
        assert raw.MP4GetTrackTimeScale(handle, track_id) == 44100


def test_tags_numeric_roundtrip(temp_mp4_file):
    with raw.MP4Create(temp_mp4_file) as handle:
        with raw.MP4TagsAlloc() as tags:
            raw.MP4TagsSetCompilation(tags, 1)
            raw.MP4TagsSetGenreType(tags, 13)
            raw.MP4TagsSetTVSeason(tags, 2)
            raw.MP4TagsSetTVEpisode(tags, 7)
            raw.MP4TagsSetPodcast(tags, 1)
            raw.MP4TagsSetHDVideo(tags, 1)
            raw.MP4TagsSetMediaType(tags, 9)
            raw.MP4TagsSetContentRating(tags, 2)
            raw.MP4TagsSetGapless(tags, 1)
            raw.MP4TagsSetITunesCountry(tags, 143441)
            raw.MP4TagsSetContentID(tags, 42)
            raw.MP4TagsSetArtistID(tags, 99)
            raw.MP4TagsSetPlaylistID(tags, 2**40)
            raw.MP4TagsSetGenreID(tags, 5)
            raw.MP4TagsSetComposerID(tags, 8)
            raw.MP4TagsSetTempo(tags, 120)
            raw.MP4TagsStore(tags, handle)
            assert tags.compilation == 1
            assert tags.genreType == 13
            assert tags.tvSeason == 2
            assert tags.tvEpisode == 7
            assert tags.podcast == 1
            assert tags.playlistID == 2**40

    with raw.MP4Read(temp_mp4_file) as handle:
        with raw.MP4TagsAlloc() as tags:
            raw.MP4TagsFetch(tags, handle)
            assert tags.compilation == 1
            assert tags.genreType == 13
            assert tags.tvSeason == 2
            assert tags.tvEpisode == 7
            assert tags.gapless == 1
            assert tags.contentID == 42
            assert tags.playlistID == 2**40
            assert tags.tempo == 120


def test_tags_strings_and_artwork(temp_mp4_file):
    cover = b"\xff\xd8\xff\xe0" + b"\x00" * 32
    with raw.MP4Create(temp_mp4_file) as handle:
        with raw.MP4TagsAlloc() as tags:
            raw.MP4TagsSetName(tags, "Song")
            raw.MP4TagsSetArtist(tags, "Artist")
            raw.MP4TagsSetAlbum(tags, "Album")
            raw.MP4TagsSetTrack(tags, raw.MP4TagTrack(1, 10))
            raw.MP4TagsAddArtwork(tags, cover, raw.MP4_ART_JPEG)
            raw.MP4TagsStore(tags, handle)
            assert tags.name == "Song"
            assert tags.artist == "Artist"
            assert tags.album == "Album"
            assert tags.track.index == 1
            assert tags.track.total == 10
            assert tags.artworkCount == 1
            assert tags.artwork[0].data == cover
            assert tags.artwork[0].type == raw.MP4_ART_JPEG

    with raw.MP4Read(temp_mp4_file) as handle:
        tags = raw.MP4TagsAlloc()
        raw.MP4TagsFetch(tags, handle)
        assert tags.name == "Song"
        assert tags.artist == "Artist"
        assert tags.artwork[0].data == cover
        assert raw.MP4TagsHasMetadata(tags)
        raw.MP4TagsFree(tags)
        assert not tags.is_valid()
        with pytest.raises(pymp4v2.MP4Error):
            raw.MP4TagsFetch(tags, handle)


def _audio_with_samples(
    path, payload=b"\x00\x01\x02\xff\x00", timescale=44100, duration=1024
):
    with raw.MP4Create(path) as handle:
        track_id = raw.MP4AddAudioTrack(handle, timescale, duration)
        raw.MP4WriteSample(handle, track_id, payload, duration=duration)
        return track_id


def test_nero_chapters_roundtrip(temp_mp4_file):
    payload = b"\x11\x22\x33"
    # Movie duration must equal the sum of chapter durations: Nero's last
    # chapter is "until end of movie", not the duration passed to SetChapters.
    _audio_with_samples(temp_mp4_file, payload, timescale=1000, duration=3000)
    chapters = [
        raw.MP4Chapter(1000, "Intro"),
        raw.MP4Chapter(2000, "Chapter Two"),
    ]
    with raw.MP4Modify(temp_mp4_file) as handle:
        written = raw.MP4SetChapters(handle, chapters, raw.MP4ChapterTypeNero)
        assert written == raw.MP4ChapterTypeNero
        kind, got = raw.MP4GetChapters(handle, raw.MP4ChapterTypeNero)
        assert kind == raw.MP4ChapterTypeNero
        assert [c.title for c in got] == ["Intro", "Chapter Two"]
        assert [c.duration for c in got] == [1000, 2000]


def test_qt_chapters_add_and_get(temp_mp4_file):
    with raw.MP4Create(temp_mp4_file) as handle:
        ref = raw.MP4AddAudioTrack(handle, 1000, 1000)
        raw.MP4WriteSample(handle, ref, b"\x00\x01", duration=1000)
        raw.MP4WriteSample(handle, ref, b"\x02\x03", duration=1000)
        chap_id = raw.MP4AddChapterTextTrack(handle, ref, 1000)
        raw.MP4AddChapter(handle, chap_id, 1000, "One")
        raw.MP4AddChapter(handle, chap_id, 1000, "Two")
        kind, got = raw.MP4GetChapters(handle, raw.MP4ChapterTypeQt)
        assert kind == raw.MP4ChapterTypeQt
        assert [c.title for c in got] == ["One", "Two"]


def test_set_chapters_qt_and_delete(temp_mp4_file):
    _audio_with_samples(temp_mp4_file)
    chapters = [raw.MP4Chapter(1500, "Alpha"), raw.MP4Chapter(1500, "Beta")]
    with raw.MP4Modify(temp_mp4_file) as handle:
        written = raw.MP4SetChapters(handle, chapters, raw.MP4ChapterTypeQt)
        assert written in (raw.MP4ChapterTypeQt, raw.MP4ChapterTypeAny)
        kind, got = raw.MP4GetChapters(handle, raw.MP4ChapterTypeQt)
        assert kind == raw.MP4ChapterTypeQt
        assert [c.title for c in got] == ["Alpha", "Beta"]
        deleted = raw.MP4DeleteChapters(handle, raw.MP4ChapterTypeQt)
        assert deleted in (
            raw.MP4ChapterTypeQt,
            raw.MP4ChapterTypeAny,
            raw.MP4ChapterTypeNone,
        )
        kind, got = raw.MP4GetChapters(handle, raw.MP4ChapterTypeQt)
        assert kind == raw.MP4ChapterTypeNone
        assert got == []


def test_chapters_and_samples_survive_optimize(temp_mp4_file, tmp_path):
    payload = b"\xde\xad\xbe\xef"
    _audio_with_samples(temp_mp4_file, payload, timescale=1000, duration=3500)
    chapters = [raw.MP4Chapter(1000, "Start"), raw.MP4Chapter(2500, "Middle")]
    with raw.MP4Modify(temp_mp4_file) as handle:
        raw.MP4SetChapters(handle, chapters, raw.MP4ChapterTypeNero)

    optimized = str(tmp_path / "optimized.mp4")
    assert raw.MP4Optimize(temp_mp4_file, optimized)

    with raw.MP4Read(optimized) as handle:
        track_id = raw.MP4FindTrackId(handle, 0, raw.MP4_AUDIO_TRACK_TYPE)
        sample = raw.MP4ReadSample(handle, track_id, 1)
        assert sample.data == payload
        assert sample.duration == 3500
        kind, got = raw.MP4GetChapters(handle, raw.MP4ChapterTypeNero)
        assert kind == raw.MP4ChapterTypeNero
        assert [c.title for c in got] == ["Start", "Middle"]
        assert [c.duration for c in got] == [1000, 2500]


def test_optimize_inplace_preserves_samples(temp_mp4_file):
    payload = b"\x0a\x0b\x0c"
    _audio_with_samples(temp_mp4_file, payload)
    assert raw.MP4Optimize(temp_mp4_file)
    with raw.MP4Read(temp_mp4_file) as handle:
        track_id = raw.MP4FindTrackId(handle, 0)
        assert raw.MP4ReadSample(handle, track_id, 1).data == payload


def test_get_chapters_empty(test_mp4_file):
    with raw.MP4Read(test_mp4_file) as handle:
        kind, got = raw.MP4GetChapters(handle, raw.MP4ChapterTypeAny)
        assert kind == raw.MP4ChapterTypeNone
        assert got == []


def test_id_type_aliases_are_int():
    assert raw.MP4TrackId is int
    assert raw.MP4SampleId is int
    assert raw.MP4Timestamp is int
    assert raw.MP4Duration is int
    assert raw.MP4EditId is int
    assert raw.MP4_INVALID_EDIT_ID == 0
    assert raw.MP4_CREATE_64BIT_DATA != 0
    assert raw.MP4_CREATE_64BIT_TIME != 0


def test_handle_exit_uses_close_flags(temp_mp4_file):
    handle = raw.MP4Create(temp_mp4_file, raw.MP4_CREATE_64BIT_DATA)
    handle.close_flags = raw.MP4_CLOSE_DO_NOT_COMPUTE_BITRATE
    assert handle.close_flags == raw.MP4_CLOSE_DO_NOT_COMPUTE_BITRATE
    with handle:
        assert handle.is_valid()
    assert not handle.is_valid()


def test_handle_close_none_uses_stored_flags(temp_mp4_file):
    handle = raw.MP4Create(temp_mp4_file)
    handle.close_flags = raw.MP4_CLOSE_DO_NOT_COMPUTE_BITRATE
    handle.close()
    assert not handle.is_valid()


def _ftyp_from_file(path):
    data = open(path, "rb").read()
    size = int.from_bytes(data[0:4], "big")
    assert data[4:8] == b"ftyp"
    payload = data[8:size]
    major = payload[0:4].decode("ascii")
    minor = int.from_bytes(payload[4:8], "big")
    brands = [payload[i : i + 4].decode("ascii") for i in range(8, len(payload), 4)]
    return major, minor, brands


def test_mp4createex_custom_brands(temp_mp4_file):
    brands = ["iso5", "iso6", "mp41"]
    with raw.MP4CreateEx(
        temp_mp4_file,
        majorBrand="iso5",
        minorVersion=1,
        compatibleBrands=brands,
    ) as handle:
        assert handle.is_valid()
        assert raw.MP4HaveAtom(handle, "ftyp")
        assert raw.MP4GetStringProperty(handle, "ftyp.majorBrand").rstrip("\x00") == "iso5"
        assert raw.MP4GetIntegerProperty(handle, "ftyp.minorVersion") == 1
        first = raw.MP4GetStringProperty(handle, "ftyp.compatibleBrands")
        assert first.rstrip("\x00") == brands[0]

    major, minor, on_disk = _ftyp_from_file(temp_mp4_file)
    assert major == "iso5"
    assert minor == 1
    assert on_disk == brands


def test_mp4createex_without_ftyp_or_iods(temp_mp4_file):
    with raw.MP4CreateEx(temp_mp4_file, add_ftyp=0, add_iods=0) as handle:
        assert not raw.MP4HaveAtom(handle, "ftyp")
        assert not raw.MP4HaveAtom(handle, "moov.iods")
        assert raw.MP4HaveAtom(handle, "moov")


def test_mp4createex_raises_on_bad_path(tmp_path):
    path = str(tmp_path / "missing-dir" / "out.mp4")
    with pytest.raises(pymp4v2.MP4Error):
        raw.MP4CreateEx(path)


def test_file_io_callbacks_and_provider_not_bound():
    assert hasattr(raw, "MP4CreateEx")
    for name in (
        "MP4CreateCallbacks",
        "MP4CreateCallbacksEx",
        "MP4ReadCallbacks",
        "MP4ModifyCallbacks",
        "MP4ReadProvider",
    ):
        assert not hasattr(raw, name)
