import os
import shutil

import pytest

import pymp4v2.raw as raw


def test_mp4read_and_mp4close(test_mp4_file):
    """Open a file for reading and close the handle."""
    handle = raw.MP4Read(test_mp4_file)
    assert handle.is_valid()

    filename = raw.MP4GetFilename(handle)
    assert filename == test_mp4_file

    raw.MP4Close(handle)
    assert not handle.is_valid()


def test_mp4create_and_mp4close(temp_mp4_file):
    """Create a file and close the handle."""
    handle = raw.MP4Create(temp_mp4_file)
    assert handle.is_valid()

    filename = raw.MP4GetFilename(handle)
    assert filename == temp_mp4_file

    raw.MP4Close(handle)
    assert not handle.is_valid()
    assert os.path.exists(temp_mp4_file)


def test_mp4modify_and_mp4close(writable_mp4):
    """Open a copy for modify so the shared fixture is not mutated."""
    handle = raw.MP4Modify(writable_mp4)
    assert handle.is_valid()

    filename = raw.MP4GetFilename(handle)
    assert filename == writable_mp4

    raw.MP4Close(handle)
    assert not handle.is_valid()


def test_mp4info(test_mp4_file):
    """Read a non-empty file summary from an open handle."""
    handle = raw.MP4Read(test_mp4_file)
    info = raw.MP4Info(handle)
    assert info is not None
    assert isinstance(info, str)
    assert len(info) > 0
    raw.MP4Close(handle)


def test_mp4info_with_track_id(test_mp4_file):
    """Unknown track id yields None from MP4Info."""
    handle = raw.MP4Read(test_mp4_file)
    info = raw.MP4Info(handle, 9999)
    assert info is None
    raw.MP4Close(handle)


def test_mp4fileinfo(test_mp4_file):
    """Read a non-empty file summary by path."""
    info = raw.MP4FileInfo(test_mp4_file)
    assert info is not None
    assert isinstance(info, str)
    assert len(info) > 0


def test_mp4fileinfo_with_track_id(test_mp4_file):
    """Unknown track id yields None from MP4FileInfo."""
    info = raw.MP4FileInfo(test_mp4_file, 9999)
    assert info is None


def test_mp4optimize(test_mp4_file, tmp_path):
    """Optimize in place and to a new path."""
    inplace = tmp_path / "to_optimize.mp4"
    shutil.copy2(test_mp4_file, inplace)
    assert raw.MP4Optimize(str(inplace))

    output = tmp_path / "optimized.mp4"
    assert raw.MP4Optimize(test_mp4_file, str(output))
    assert output.exists()
    assert output.stat().st_size > 0


def test_mp4dump(test_mp4_file, capfd):
    """Dump prints to stdout (implicit properties optional)."""
    raw.MP4LogSetLevel(raw.MP4_LOG_INFO)
    try:
        handle = raw.MP4Read(test_mp4_file)

        raw.MP4Dump(handle)
        dump_output = capfd.readouterr()
        assert dump_output != ""

        raw.MP4Dump(handle, True)
        dump_output_with_implicits = capfd.readouterr()
        assert dump_output_with_implicits != ""

        raw.MP4Close(handle)
    finally:
        raw.MP4LogSetLevel(raw.MP4_LOG_NONE)


def test_invalid_file_operations():
    """Closed/empty handle raises; close is a no-op."""
    handle = raw.MP4FileHandle()
    assert not handle.is_valid()

    with pytest.raises(RuntimeError):
        raw.MP4GetFilename(handle)

    raw.MP4Close(handle)


def test_context_manager(test_mp4_file):
    """with-block closes the handle on exit."""
    with raw.MP4Read(test_mp4_file) as handle:
        assert handle.is_valid()
        filename = raw.MP4GetFilename(handle)
        assert filename == test_mp4_file

    assert not handle.is_valid()


def test_mp4create_with_flags(temp_mp4_file):
    """MP4Create accepts a flags bitmask (0 is the default)."""
    handle = raw.MP4Create(temp_mp4_file, 0)
    assert handle.is_valid()
    raw.MP4Close(handle)
    assert os.path.exists(temp_mp4_file)


def test_mp4createex_defaults(temp_mp4_file):
    """MP4CreateEx with defaults creates ftyp (mp42) and iods like MP4Create."""
    handle = raw.MP4CreateEx(temp_mp4_file)
    assert handle.is_valid()
    assert raw.MP4HaveAtom(handle, "ftyp")
    assert raw.MP4HaveAtom(handle, "moov.iods")
    assert raw.MP4GetStringProperty(handle, "ftyp.majorBrand").rstrip("\x00") == "mp42"
    raw.MP4Close(handle)
    assert os.path.exists(temp_mp4_file)
    assert os.path.getsize(temp_mp4_file) > 0


def test_mp4modify_with_flags(writable_mp4):
    """MP4Modify accepts a flags bitmask (ignored by mp4v2, default 0)."""
    handle = raw.MP4Modify(writable_mp4, 0)
    assert handle.is_valid()
    raw.MP4Close(handle)


def test_double_close(test_mp4_file):
    """Closing twice is safe."""
    handle = raw.MP4Read(test_mp4_file)
    raw.MP4Close(handle)
    assert not handle.is_valid()
    raw.MP4Close(handle)
    assert not handle.is_valid()
