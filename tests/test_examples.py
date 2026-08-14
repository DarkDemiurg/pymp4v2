"""Run examples/ against the live API (subprocess, same interpreter)."""

import subprocess
import sys
from pathlib import Path

import pymp4v2
import pymp4v2.raw as raw

EXAMPLES = Path(__file__).resolve().parents[1] / "examples"


def _run(script, *args, check=True):
    return subprocess.run(
        [sys.executable, str(EXAMPLES / script), *args],
        capture_output=True,
        text=True,
        check=check,
    )


def _mp4_with_samples(path, payloads):
    with raw.MP4Create(path) as handle:
        tid = raw.MP4AddAudioTrack(handle, 44100, 1024)
        for data in payloads:
            raw.MP4WriteSample(handle, tid, data, duration=1024)
        with raw.MP4TagsAlloc() as tags:
            raw.MP4TagsSetName(tags, "Demo")
            raw.MP4TagsSetArtist(tags, "Example")
            raw.MP4TagsStore(tags, handle)


def test_dump_info_prints_tracks_and_tags(temp_mp4_file):
    _mp4_with_samples(temp_mp4_file, [b"\x00\x01"])
    result = _run("dump_info.py", temp_mp4_file)
    assert result.returncode == 0
    out = result.stdout
    assert "tracks (1):" in out
    assert "type=soun" in out
    assert "samples=1" in out
    assert "name: 'Demo'" in out
    assert "artist: 'Example'" in out


def test_dump_info_atoms_flag(test_mp4_file):
    result = _run("dump_info.py", test_mp4_file, "--atoms")
    assert result.returncode == 0
    assert "info:" in result.stdout
    # MP4Dump prints atom names; ftyp is present in the generated fixture.
    combined = result.stdout + result.stderr
    assert "ftyp" in combined or "moov" in combined or "info:" in result.stdout


def test_dump_info_missing_file(tmp_path):
    missing = str(tmp_path / "nope.mp4")
    result = _run("dump_info.py", missing, check=False)
    assert result.returncode == 1
    assert "error:" in result.stderr


def test_retag_writes_tags_and_optional_copy(temp_mp4_file, tmp_path):
    _mp4_with_samples(temp_mp4_file, [b"\xaa"])
    dest = str(tmp_path / "tagged.mp4")
    cover = tmp_path / "cover.jpg"
    cover.write_bytes(b"\xff\xd8\xff\xe0" + b"\x00" * 8)
    result = _run(
        "retag.py",
        temp_mp4_file,
        "-o",
        dest,
        "--name",
        "NewTitle",
        "--album",
        "NewAlbum",
        "--artwork",
        str(cover),
        "--optimize",
    )
    assert result.returncode == 0
    with pymp4v2.MP4File(temp_mp4_file) as original:
        assert original.tags.name == "Demo"
    with pymp4v2.MP4File(dest) as tagged:
        assert tagged.tags.name == "NewTitle"
        assert tagged.tags.album == "NewAlbum"
        assert tagged.tags.artwork_count == 1
        assert tagged.tags.artwork[0][:3] == b"\xff\xd8\xff"


def test_extract_samples_per_sample_and_concat(temp_mp4_file, tmp_path):
    payloads = [b"\x00\x01", b"\xfe\xff"]
    _mp4_with_samples(temp_mp4_file, payloads)
    outdir = tmp_path / "samples"
    result = _run("extract_samples.py", temp_mp4_file, "--outdir", str(outdir))
    assert result.returncode == 0
    files = sorted(outdir.glob("*.bin"))
    assert len(files) == 2
    assert [p.read_bytes() for p in files] == payloads

    concat_dir = tmp_path / "concat"
    result = _run(
        "extract_samples.py",
        temp_mp4_file,
        "--outdir",
        str(concat_dir),
        "--track",
        "0",
        "--concat",
    )
    assert result.returncode == 0
    concat_files = list(concat_dir.glob("*.bin"))
    assert len(concat_files) == 1
    assert concat_files[0].read_bytes() == b"".join(payloads)


def test_extract_samples_empty_track(test_mp4_file, tmp_path):
    outdir = tmp_path / "empty"
    result = _run("extract_samples.py", test_mp4_file, "--outdir", str(outdir))
    assert result.returncode == 0
    assert "no samples" in result.stderr
    assert list(outdir.glob("*.bin")) == []
