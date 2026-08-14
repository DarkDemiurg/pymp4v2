#!/usr/bin/env python3
"""Set iTMF tags on an existing MP4 (modify in place).

Only fields passed on the command line are changed. Artwork is JPEG/PNG/GIF
bytes; the type is inferred from the file suffix. Pass ``--optimize`` to
rewrite the file layout after storing tags.
"""

from __future__ import annotations

import argparse
import shutil
import sys
from pathlib import Path
from typing import Optional, Sequence

import pymp4v2
import pymp4v2.raw as raw

_ART_BY_SUFFIX = {
    ".jpg": raw.MP4_ART_JPEG,
    ".jpeg": raw.MP4_ART_JPEG,
    ".png": raw.MP4_ART_PNG,
    ".gif": raw.MP4_ART_GIF,
    ".bmp": raw.MP4_ART_BMP,
}


def retag(
    path: str,
    *,
    name: Optional[str] = None,
    artist: Optional[str] = None,
    album: Optional[str] = None,
    album_artist: Optional[str] = None,
    genre: Optional[str] = None,
    comments: Optional[str] = None,
    artwork: Optional[str] = None,
    optimize: bool = False,
) -> None:
    with pymp4v2.MP4File(path, "a") as mp4:
        tags = mp4.tags
        if name is not None:
            tags.name = name
        if artist is not None:
            tags.artist = artist
        if album is not None:
            tags.album = album
        if album_artist is not None:
            tags.album_artist = album_artist
        if genre is not None:
            tags.genre = genre
        if comments is not None:
            tags.comments = comments
        if artwork is not None:
            art_path = Path(artwork)
            suffix = art_path.suffix.lower()
            art_type = _ART_BY_SUFFIX.get(suffix, raw.MP4_ART_UNDEFINED)
            tags.add_artwork(art_path.read_bytes(), art_type)
        if optimize:
            mp4.optimize()


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("path", help="MP4 file to modify")
    parser.add_argument("--name", help="title / name tag")
    parser.add_argument("--artist", help="artist tag")
    parser.add_argument("--album", help="album tag")
    parser.add_argument("--album-artist", dest="album_artist", help="album artist tag")
    parser.add_argument("--genre", help="genre tag")
    parser.add_argument("--comments", help="comments tag")
    parser.add_argument("--artwork", help="path to cover image (jpeg/png/gif/bmp)")
    parser.add_argument(
        "--optimize",
        action="store_true",
        help="call MP4File.optimize() after writing tags",
    )
    parser.add_argument(
        "-o",
        "--output",
        help="copy path to OUTPUT first and modify the copy (leave the original unchanged)",
    )
    args = parser.parse_args(argv)
    target = args.path
    if args.output:
        shutil.copy2(args.path, args.output)
        target = args.output
    try:
        retag(
            target,
            name=args.name,
            artist=args.artist,
            album=args.album,
            album_artist=args.album_artist,
            genre=args.genre,
            comments=args.comments,
            artwork=args.artwork,
            optimize=args.optimize,
        )
    except pymp4v2.MP4Error as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
