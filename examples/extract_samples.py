#!/usr/bin/env python3
"""Extract track samples as raw bytes.

Sample indices on ``MP4File.tracks`` are 0-based. Without ``--concat``, each
sample is written as ``{stem}_t{id}_s{index:04d}.bin``. With ``--concat``,
one file per track is written (samples concatenated in order).
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path
from typing import Optional, Sequence

import pymp4v2


def extract(
    path: str,
    outdir: str,
    *,
    track_index: Optional[int] = None,
    concat: bool = False,
) -> list[str]:
    dest = Path(outdir)
    dest.mkdir(parents=True, exist_ok=True)
    stem = Path(path).stem
    written: list[str] = []
    with pymp4v2.MP4File(path) as mp4:
        if track_index is None:
            tracks = list(enumerate(mp4.tracks))
        else:
            tracks = [(track_index, mp4.tracks[track_index])]
        for i, track in tracks:
            if concat:
                payload = b"".join(sample.data for sample in track)
                out = dest / f"{stem}_t{track.id}.bin"
                out.write_bytes(payload)
                written.append(str(out))
            else:
                for s, sample in enumerate(track):
                    out = dest / f"{stem}_t{track.id}_s{s:04d}.bin"
                    out.write_bytes(sample.data)
                    written.append(str(out))
            if track.sample_count == 0:
                print(
                    f"track [{i}] id={track.id} type={track.type}: no samples",
                    file=sys.stderr,
                )
    return written


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("path", help="MP4 file to read")
    parser.add_argument(
        "--outdir",
        default=".",
        help="directory for extracted samples (default: current directory)",
    )
    parser.add_argument(
        "--track",
        type=int,
        default=None,
        metavar="INDEX",
        help="0-based index into MP4File.tracks (default: all tracks)",
    )
    parser.add_argument(
        "--concat",
        action="store_true",
        help="write one file per track instead of one file per sample",
    )
    args = parser.parse_args(argv)
    try:
        written = extract(
            args.path, args.outdir, track_index=args.track, concat=args.concat
        )
    except (pymp4v2.MP4Error, IndexError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    for path in written:
        print(path)
    return 0


if __name__ == "__main__":
    sys.exit(main())
