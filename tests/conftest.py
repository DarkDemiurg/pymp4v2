import shutil
import struct

import pytest

import pymp4v2.raw as raw


def pytest_configure():
    raw.MP4LogSetLevel(raw.MP4_LOG_NONE)


def _box(typ: bytes, payload: bytes) -> bytes:
    return struct.pack(">I", 8 + len(payload)) + typ + payload


def _full_box(typ: bytes, version: int, flags: int, payload: bytes) -> bytes:
    header = struct.pack(">B", version) + flags.to_bytes(3, "big")
    return _box(typ, header + payload)


def minimal_mp4_bytes() -> bytes:
    """ISO BMFF with ftyp/moov/trak so MP4Modify and MP4Optimize have a trak."""
    ftyp = _box(b"ftyp", b"isom" + struct.pack(">I", 0x200) + b"isomiso2mp41")

    mvhd = _full_box(
        b"mvhd",
        0,
        0,
        struct.pack(">IIII", 0, 0, 1000, 0)
        + struct.pack(">IH", 0x00010000, 0x0100)
        + b"\x00" * 10
        + struct.pack(">9I", 0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000)
        + b"\x00" * 24
        + struct.pack(">I", 2),
    )
    tkhd = _full_box(
        b"tkhd",
        0,
        0x3,
        struct.pack(">IIIII", 0, 0, 1, 0, 0)
        + b"\x00" * 8
        + struct.pack(">HHHH", 0, 0, 0x0100, 0)
        + struct.pack(">9I", 0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000)
        + struct.pack(">II", 0, 0),
    )
    mdhd = _full_box(
        b"mdhd",
        0,
        0,
        struct.pack(">IIIIHH", 0, 0, 1000, 0, 0x55C4, 0),
    )
    hdlr = _full_box(
        b"hdlr",
        0,
        0,
        struct.pack(">I", 0) + b"vide" + b"\x00" * 12 + b"VideoHandler\x00",
    )
    vmhd = _full_box(b"vmhd", 0, 1, struct.pack(">HHHH", 0, 0, 0, 0))
    url_ = _full_box(b"url ", 0, 1, b"")
    dref = _full_box(b"dref", 0, 0, struct.pack(">I", 1) + url_)
    dinf = _box(b"dinf", dref)
    stsd = _full_box(b"stsd", 0, 0, struct.pack(">I", 0))
    stts = _full_box(b"stts", 0, 0, struct.pack(">I", 0))
    stsc = _full_box(b"stsc", 0, 0, struct.pack(">I", 0))
    stsz = _full_box(b"stsz", 0, 0, struct.pack(">II", 0, 0))
    stco = _full_box(b"stco", 0, 0, struct.pack(">I", 0))
    stbl = _box(b"stbl", stsd + stts + stsc + stsz + stco)
    minf = _box(b"minf", vmhd + dinf + stbl)
    mdia = _box(b"mdia", mdhd + hdlr + minf)
    trak = _box(b"trak", tkhd + mdia)
    moov = _box(b"moov", mvhd + trak)
    mdat = _box(b"mdat", b"\x00")
    return ftyp + moov + mdat


@pytest.fixture(scope="session")
def test_mp4_file(tmp_path_factory):
    """Tiny legal MP4 generated in-process; no vendored sample.mp4 required."""
    path = tmp_path_factory.mktemp("fixtures") / "sample.mp4"
    path.write_bytes(minimal_mp4_bytes())
    return str(path)


@pytest.fixture
def writable_mp4(test_mp4_file, tmp_path):
    """Copy of the session fixture so MP4Modify cannot mutate the shared file."""
    dest = tmp_path / "writable.mp4"
    shutil.copy2(test_mp4_file, dest)
    return str(dest)


@pytest.fixture
def temp_mp4_file(tmp_path):
    """Path for a new MP4 that does not exist yet (MP4Create target)."""
    return str(tmp_path / "new.mp4")
