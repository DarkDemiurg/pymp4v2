#!/usr/bin/env python3
"""Print a textual summary of an MP4: info, tracks, and iTMF tags.

Uses the pythonic ``MP4File`` API. Pass ``--atoms`` to also print the raw
atom dump (``MP4Dump`` writes to stdout; it does not return a string).
"""

from __future__ import annotations

import argparse
import sys
from typing import Optional, Sequence, TextIO

import pymp4v2
import pymp4v2.raw as raw


def dump(path: str, *, atoms: bool = False, file: TextIO = sys.stdout) -> None:
    with pymp4v2.MP4File(path) as mp4:
        print(f"file: {mp4.filename}", file=file)
        print(f"mode: {mp4.mode}", file=file)
        print(f"duration: {mp4.duration} timescale: {mp4.timescale}", file=file)
        print(file=file)
        print("info:", file=file)
        print(mp4.info, file=file)
        print(f"tracks ({len(mp4.tracks)}):", file=file)
        for i, track in enumerate(mp4.tracks):
            print(
                f"  [{i}] id={track.id} type={track.type} "
                f"duration={track.duration} timescale={track.timescale} "
                f"samples={track.sample_count} "
                f"bitrate={track.bitrate} fps={track.frame_rate} "
                f"channels={track.audio_channels} "
                f"size={track.width}x{track.height} "
                f"lang={track.language!r} name={track.name!r}",
                file=file,
            )
        tags = mp4.tags
        print("tags:", file=file)
        for field in (
            "name",
            "artist",
            "album_artist",
            "album",
            "genre",
            "comments",
        ):
            print(f"  {field}: {getattr(tags, field)!r}", file=file)
        print(f"  artwork: {tags.artwork_count} item(s)", file=file)
        if atoms:
            with raw.MP4Read(path) as handle:
                raw.MP4Dump(handle)


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("path", help="MP4 file to inspect")
    parser.add_argument(
        "--atoms",
        action="store_true",
        help="also print MP4Dump atom listing to stdout",
    )
    args = parser.parse_args(argv)
    try:
        dump(args.path, atoms=args.atoms)
    except pymp4v2.MP4Error as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
