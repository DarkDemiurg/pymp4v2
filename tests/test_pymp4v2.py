import os
import shutil
from importlib.metadata import version

import pytest

import pymp4v2


def test_version_matches_package_metadata():
    assert isinstance(pymp4v2.__version__, str)
    assert pymp4v2.__version__ == version("pymp4v2")


def test_read_mode(test_mp4_file):
    mp4_file = pymp4v2.MP4File(test_mp4_file, "r")
    assert mp4_file.is_open()
    assert mp4_file.mode == "r"
    assert mp4_file.filename == test_mp4_file
    assert mp4_file.get_track_count() == 1
    info = mp4_file.get_info()
    assert isinstance(info, str)
    assert len(info) > 0
    mp4_file.close()
    assert not mp4_file.is_open()


def test_default_mode_is_read(test_mp4_file):
    mp4_file = pymp4v2.MP4File(test_mp4_file)
    assert mp4_file.mode == "r"
    mp4_file.close()


def test_create_mode(temp_mp4_file):
    mp4_file = pymp4v2.MP4File(temp_mp4_file, "w")
    assert mp4_file.is_open()
    assert mp4_file.mode == "w"
    mp4_file.close()
    assert os.path.exists(temp_mp4_file)


def test_modify_modes_use_a_copy(test_mp4_file, tmp_path):
    for mode in ("a", "r+"):
        dest = tmp_path / f"writable-{mode.replace('+', 'plus')}.mp4"
        shutil.copy2(test_mp4_file, dest)
        mp4_file = pymp4v2.MP4File(str(dest), mode)
        assert mp4_file.is_open()
        assert mp4_file.mode == mode
        mp4_file.close()


def test_unknown_mode_includes_filename(test_mp4_file):
    with pytest.raises(RuntimeError, match=test_mp4_file) as exc:
        pymp4v2.MP4File(test_mp4_file, "x")
    assert "Unsupported mode" in str(exc.value)


def test_missing_file_includes_filename(tmp_path):
    missing = str(tmp_path / "does-not-exist.mp4")
    with pytest.raises(RuntimeError, match=missing):
        pymp4v2.MP4File(missing, "r")


def test_save_read_only_is_noop(test_mp4_file):
    mp4_file = pymp4v2.MP4File(test_mp4_file, "r")
    mp4_file.save()
    assert mp4_file.is_open()
    mp4_file.close()


def test_save_keeps_writable_file_open(temp_mp4_file):
    mp4_file = pymp4v2.MP4File(temp_mp4_file, "w")
    mp4_file.save()
    assert mp4_file.is_open()
    assert mp4_file.get_track_count() == 0
    mp4_file.close()


def test_save_on_closed_file_raises(test_mp4_file):
    mp4_file = pymp4v2.MP4File(test_mp4_file)
    mp4_file.close()
    with pytest.raises(RuntimeError, match="closed"):
        mp4_file.save()


def test_closed_file_raises_on_track_count_and_info(test_mp4_file):
    mp4_file = pymp4v2.MP4File(test_mp4_file)
    mp4_file.close()
    with pytest.raises(RuntimeError, match="closed"):
        mp4_file.get_track_count()
    with pytest.raises(RuntimeError, match="closed"):
        mp4_file.get_info()


def test_context_manager(test_mp4_file):
    with pymp4v2.MP4File(test_mp4_file) as mp4_file:
        assert mp4_file.is_open()
        assert mp4_file.get_track_count() == 1
    assert not mp4_file.is_open()


def test_double_close(test_mp4_file):
    mp4_file = pymp4v2.MP4File(test_mp4_file)
    mp4_file.close()
    mp4_file.close()
    assert not mp4_file.is_open()
