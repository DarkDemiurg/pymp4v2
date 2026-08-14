# Changelog

## [Unreleased]

## [0.1.13] - 2026-08-14
### Added
- `raw.MP4CreateEx`: create with ftyp major/compatible brands, minor version, and optional iods. Custom I/O callbacks and `MP4ReadProvider` stay unbound.
- Sample timing/copy: `MP4ReadSampleFromTime`, `MP4GetSampleIdFromTime`, `MP4GetSampleTime`, `MP4GetSampleDuration`, `MP4GetSampleSync`, `MP4CopySample` (`bytes`, `MP4Error` on C failure).
- File/track properties: generic Get/Set Integer/Float/String/Bytes, `MP4ChangeMovieTimeScale`, bitrate / frame rate / audio channels, ES configuration (`bytes`), `MP4HaveTrackAtom`.
- Remaining iTMF convenience scalars (`compilation`, `genreType`, `tvSeason`, …) on `raw.MP4Tags` and high-level `Tags`.
- High-level `MP4File.duration` / `.timescale`; `Track.bitrate` / `.frame_rate` / `.audio_channels`.
- `MP4FileHandle.close_flags`: used by `close()` with no args, `__exit__`, and the destructor.
- Raw constants: `MP4_CREATE_64BIT_DATA`, `MP4_INVALID_EDIT_ID`.
- `examples/`: `dump_info.py`, `retag.py`, `extract_samples.py` (live `MP4File` / `raw` API).
- PEP 561 stubs in the wheel (`py.typed`, `__init__.pyi`, `raw.pyi`); extension is `pymp4v2._pymp4v2`.
- `NOTICE` and a copy of mp4v2's MPL 1.1 `COPYING` (`licenses/mp4v2/COPYING`); wheels include both.
- cibuildwheel: manylinux (x86_64, aarch64), Windows AMD64, macOS x86_64 and arm64 (CPython 3.9–3.13).
- Release workflow on `v*` tags: wheels + sdist + GitHub Release. Not published to PyPI (no Trusted Publishing).
- Raw track API: `MP4GetNumberOfTracks`, `MP4FindTrackId`, `MP4GetTrackType`, `MP4DeleteTrack`, `MP4AddAudioTrack`, `MP4AddVideoTrack`.
- File/track properties: duration, timescale, video width/height, language, track name, `MP4HaveAtom`.
- Sample I/O: `MP4ReadSample` / `MP4WriteSample` (`bytes`), `MP4GetSampleSize`, `MP4GetTrackNumberOfSamples`, `MP4GetTrackMaxSampleSize`.
- iTMF tags: `MP4TagsAlloc` / `Fetch` / `Store` / `Free`, string setters, artwork as copied `bytes`.
- Chapters: `MP4GetChapters` / `MP4SetChapters` / `MP4AddChapter` / `MP4AddChapterTextTrack` / `MP4AddNeroChapter` / `MP4ConvertChapters` / `MP4DeleteChapters`.
- Pythonic `MP4File.tracks` (`Track`, 0-based samples), `MP4File.tags` (`Tags`, artwork as `list[bytes]`), `MP4File.optimize()`.
- `pymp4v2.MP4Error` (`RuntimeError` subclass). Long C calls release the GIL.
### Fixed
- `MP4FileHandle.__exit__` / destructor honor `close_flags` instead of always passing `0`.
- `MP4Info` / `MP4FileInfo` / `MP4File.get_info()` copy the C summary string and `MP4Free` it.
- `MP4File` honors `mode` (`r` / `w` / `a` / `r+`), includes the filename in errors, uses the RAII handle wrapper, and implements a context manager.
- `MP4File.save()` persists via close+reopen instead of leaving a closed handle.
- Tests generate a tiny MP4 fixture instead of depending on gitignored `tests/sample.mp4`; modify tests copy first.
- `MP4TagArtwork` no longer exposes a raw `void*` (data is copied `bytes`).
### Changed
- `raw.MP4TrackId` (and SampleId / Timestamp / Duration / EditId) are `int` aliases, not `typing.NewType`.
- Remaining C API (hint/streaming, generic iTMF, profile-level setters) is documented as Alpha out of scope.
- Dropped the `Private :: Do Not Upload` classifier; license is `MIT AND MPL-1.1`.
- README, usage, and API docs describe only the shipped surface (Alpha, not full mp4v2).
- One build path: scikit-build-core + CMake FetchContent; mp4v2 v2.1.3 is a SHA256-pinned release tarball (no `git clone` in `setup.py`).
- Version lives only in `pyproject.toml`; the extension exports `pymp4v2.__version__`.
- Single `tox.ini`: lint/format run on `tests/`, `tox -e docs` runs `mkdocs build` (no `gh-deploy`).
- CI matrix includes `macos-latest`.
- `MP4Dump` / `MP4Optimize` raise `MP4Error` on failure instead of returning `False`.

## [0.1.12] - 2025-09-17
### Added
- Docker build.
### Changed
- Fix mkdocs build.

## [0.1.11] - 2025-09-16
### Changed
- Fix tox envs.

## [0.1.10] - 2025-09-16
### Changed
- Fix deploy gh-pages.

## [0.1.9] - 2025-09-16
### Changed
- Deploy gh-pages.

## [0.1.8] - 2025-09-16
### Changed
- CHANGELOG.md.

## [0.1.7] - 2025-09-16
### Added
- GitHub CI action for docs.

## [0.1.6] - 2025-09-16
### Added
- mkdocs support.

## [0.1.5] - 2025-09-16
### Added
- Initial docs.

## [0.1.4] - 2025-09-16
### Added
- CONTRIBUTING.md.

## [0.1.3] - 2025-09-16
### Added
- Comments for raw functions.

## [0.1.2] - 2025-09-16
### Changed
- Fix msvc-dev-cmd arg for CI.

## [0.1.1] - 2025-09-16
### Removed
- Pipfile.

## [0.1.1] - 2025-09-16
### Added
- bump4version support.

## [0.1.0] - 2025-09-15
### Added
- First release on GitHub.
