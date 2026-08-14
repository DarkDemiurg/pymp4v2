import pytest

import pymp4v2
import pymp4v2.raw as raw


def test_tracks_on_fixture(test_mp4_file):
    with pymp4v2.MP4File(test_mp4_file) as mp4:
        assert len(mp4.tracks) == 1
        assert mp4.get_track_count() == 1
        track = mp4.tracks[0]
        assert track.id != raw.MP4_INVALID_TRACK_ID
        assert track.type == raw.MP4_VIDEO_TRACK_TYPE
        assert track.sample_count == 0
        assert track.duration == 0
        assert len(track) == 0
        assert track.timescale == 1000
        assert mp4.timescale == 1000
        assert isinstance(mp4.duration, int)
        assert track.bitrate >= 0
        assert track.audio_channels is None
        with pytest.raises(IndexError):
            track.read_sample(0)
        with pytest.raises(IndexError):
            mp4.tracks[5]


def test_track_sample_iteration(temp_mp4_file):
    payloads = [b"\x00\x01", b"\xfe\xff"]
    with raw.MP4Create(temp_mp4_file) as handle:
        tid = raw.MP4AddAudioTrack(handle, 44100, 1024)
        for data in payloads:
            raw.MP4WriteSample(handle, tid, data, duration=1024)

    with pymp4v2.MP4File(temp_mp4_file) as mp4:
        track = mp4.tracks[0]
        assert track.type == raw.MP4_AUDIO_TRACK_TYPE
        assert track.sample_count == 2
        first = track.read_sample(0)
        assert first.data == payloads[0]
        assert first.duration == 1024
        assert isinstance(first, pymp4v2.Sample)
        assert track.audio_channels is not None
        assert track.audio_channels >= 0
        samples = list(track)
        assert [s.data for s in samples] == payloads
        assert track[-1].data == payloads[1]


def test_tags_roundtrip_and_artwork(temp_mp4_file):
    cover = b"\xff\xd8\xff\xe0" + b"\x00" * 16
    with pymp4v2.MP4File(temp_mp4_file, "w") as mp4:
        with mp4.tags as tags:
            tags.name = "Song"
            tags.artist = "Artist"
            tags.album = "Album"
            tags.album_artist = "AA"
            tags.tv_show = "Show"
            tags.compilation = 1
            tags.tv_season = 3
            tags.tempo = 110
            tags.track = raw.MP4TagTrack(2, 12)
            tags.add_artwork(cover, raw.MP4_ART_JPEG)
            assert tags.name == "Song"
            assert tags.artwork == [cover]
            assert tags.artwork_count == 1
            assert tags.tv_show == "Show"
            assert tags.compilation == 1
            assert tags.track.index == 2

    with pymp4v2.MP4File(temp_mp4_file) as mp4:
        assert mp4.tags.name == "Song"
        assert mp4.tags.artist == "Artist"
        assert mp4.tags.album == "Album"
        assert mp4.tags.album_artist == "AA"
        assert mp4.tags.tv_show == "Show"
        assert mp4.tags.compilation == 1
        assert mp4.tags.tv_season == 3
        assert mp4.tags.tempo == 110
        assert mp4.tags.track.index == 2
        assert mp4.tags.track.total == 12
        assert mp4.tags.artwork == [cover]
        with pytest.raises(pymp4v2.MP4Error, match="read-only"):
            mp4.tags.name = "Nope"


def test_tags_store_on_file_close(temp_mp4_file):
    with pymp4v2.MP4File(temp_mp4_file, "w") as mp4:
        mp4.tags.name = "Auto"
    with pymp4v2.MP4File(temp_mp4_file) as mp4:
        assert mp4.tags.name == "Auto"


def test_optimize_method_roundtrip(temp_mp4_file, tmp_path):
    payload = b"\x01\x02\x03\x04"
    with raw.MP4Create(temp_mp4_file) as handle:
        tid = raw.MP4AddAudioTrack(handle, 44100, 512)
        raw.MP4WriteSample(handle, tid, payload, duration=512)

    dest = str(tmp_path / "out.mp4")
    with pymp4v2.MP4File(temp_mp4_file, "a") as mp4:
        mp4.tags.name = "Opt"
        mp4.optimize(dest)
        assert mp4.is_open()

    with pymp4v2.MP4File(dest) as mp4:
        assert mp4.tags.name == "Opt"
        track = mp4.tracks[0]
        assert track[0].data == payload


def test_closed_tracks_raise(test_mp4_file):
    mp4 = pymp4v2.MP4File(test_mp4_file)
    mp4.close()
    with pytest.raises(pymp4v2.MP4Error, match="closed"):
        len(mp4.tracks)
    with pytest.raises(pymp4v2.MP4Error, match="closed"):
        mp4.tags.name
