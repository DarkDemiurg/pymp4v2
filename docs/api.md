## API Reference

**Alpha.** Only the functions and types below are shipped. This is not a complete
mp4v2 binding. `pymp4v2.__version__` is the package version from `pyproject.toml`.
Wheels include PEP 561 stubs (`pymp4v2/py.typed`, `__init__.pyi`, `raw.pyi`).

### `pymp4v2.MP4File`

High-level owner of an `MP4FileHandle`.

| Member | Description |
| --- | --- |
| `MP4File(filename, mode="r")` | `"r"` → `MP4Read`, `"w"` → `MP4Create`, `"a"` / `"r+"` → `MP4Modify`. Raises `MP4Error` (subclass of `RuntimeError`, with filename) on unknown mode or open failure. |
| `filename` | Path passed to the constructor. |
| `mode` | Mode string passed to the constructor. |
| `is_open()` | Whether the handle is still valid. |
| `get_info()` / `info` | Textual summary (`str`). Raises if closed. |
| `get_track_count()` | Number of tracks. Raises if closed (does not return `-1`). |
| `duration` / `timescale` | Movie timeline (timescale units / ticks per second). |
| `tracks` | Sequence of `Track` (indexable, iterable). Sample indices on `Track` are 0-based. |
| `tags` | `Tags` snapshot (iTMF). String properties (`name`, `artist`, `album`, …); `artwork` is `list[bytes]`. Dirty tags are stored on `Tags` context exit and on `MP4File.close()`. |
| `optimize(new_filename=None)` | Close (persist), `MP4Optimize`, reopen if it was open. |
| `save()` | Persist pending writes by close+reopen (`MP4Modify`). No-op for `"r"`. Raises `MP4Error` if closed. |
| `close()` | Close the handle (flags `0`). Create/modify handles write pending data here; dirty tags are stored first. |
| `__enter__` / `__exit__` | Context manager; `__exit__` calls `close()`. |

### `pymp4v2.Track`

Returned from `MP4File.tracks[i]`. Indices for samples are **0-based** (raw `MP4ReadSample` is 1-based).

| Member | Description |
| --- | --- |
| `id`, `type`, `duration`, `timescale`, `sample_count` | Track identity and timeline. |
| `language`, `name` | `str` or `None` if unset. |
| `width`, `height` | Video pixels (0 on non-video). |
| `bitrate` | Average bits per second (0 if unknown). |
| `frame_rate` | Average frames per second (0 if unknown / non-video). |
| `audio_channels` | Channel count, or `None` if the track is not audio. |
| `read_sample(index)` / `[index]` | `Sample` (`data` bytes + timing). |
| `__len__` / iteration | Over samples. |

### `pymp4v2.Tags`

Returned from `MP4File.tags`. Fetch happens on first access.

| Member | Description |
| --- | --- |
| `name`, `artist`, `album`, `album_artist`, `composer`, `comments`, `genre`, … | String properties; `None` clears. Read-only files raise on set. |
| `tempo`, `compilation`, `tv_season`, `track`, `disk`, … | Numeric / struct iTMF fields (`track`/`disk` are `raw.MP4TagTrack` / `MP4TagDisk`). |
| `artwork` | `list[bytes]` copies. |
| `add_artwork(data, type=MP4_ART_UNDEFINED)` | `data` is `bytes`. |
| `store()` / `fetch()` | Write / reload the snapshot. |
| `__enter__` / `__exit__` | Stores if dirty on exit. |

`pymp4v2.Sample` is an alias of `raw.MP4Sample`.

### `pymp4v2.MP4Error`

Subclass of `RuntimeError`. Raised when a C call returns `false` or `MP4_INVALID_*`,
or when a closed handle is used. Predicates such as `MP4HaveAtom` return `bool`
instead of raising.

### `pymp4v2.raw`

1:1 wrappers around the bound C API. Handles are `MP4FileHandle` objects (RAII,
context manager). Open failures and C errors raise `MP4Error`. Long calls
(`MP4Read` / `Create` / `CreateEx` / `Modify` / `Close` / `Dump` / `Optimize` / `Info`,
track/sample/tag I/O) release the GIL. `MP4FileHandle.close_flags` (default `0`)
is used by `close()` with no args, `__exit__`, and the destructor; `MP4Close(h, flags)`
still takes an explicit C-style flags argument.

| Function | Returns | Notes |
| --- | --- | --- |
| `MP4Read(fileName)` | `MP4FileHandle` | Read-only open. |
| `MP4Create(fileName, flags=0)` | `MP4FileHandle` | Create/truncate. Flags: `MP4_CREATE_64BIT_DATA`, `MP4_CREATE_64BIT_TIME`. |
| `MP4CreateEx(fileName, flags=0, add_ftyp=1, add_iods=1, majorBrand=None, minorVersion=0, compatibleBrands=())` | `MP4FileHandle` | Create with ftyp brands / iods. `None` majorBrand keeps mp4v2 defaults (`mp42` / `isom`). Count is the sequence length. |
| `MP4Modify(fileName, flags=0)` | `MP4FileHandle` | Read/write open. `flags` currently ignored by mp4v2. |
| `MP4Close(hFile, flags=0)` | `None` | Writes pending data for create/modify handles. Flag: `MP4_CLOSE_DO_NOT_COMPUTE_BITRATE`. |
| `MP4GetFilename(hFile)` | `str` | Path associated with the handle. |
| `MP4Info(hFile, trackId=MP4_INVALID_TRACK_ID)` | `str` or `None` | Wrapper copies the C string and frees it. |
| `MP4FileInfo(fileName, trackId=MP4_INVALID_TRACK_ID)` | `str` or `None` | Same as `MP4Info` without an open handle. |
| `MP4Dump(hFile, dumpImplicits=False)` | `bool` | Prints to stdout (or the log callback); does **not** return a dump string. Raises `MP4Error` on failure. |
| `MP4Optimize(fileName, newFileName=None)` | `bool` | Rewrite with interleaved samples / moov at the front. Raises `MP4Error` on failure. |
| `MP4LogSetLevel(verbosity)` | `None` | `MP4LogLevel` enum. |
| `MP4GetNumberOfTracks(hFile, type=None, subType=0)` | `int` | `type` is a four-cc such as `MP4_VIDEO_TRACK_TYPE`. |
| `MP4FindTrackId(hFile, index, type=None, subType=0)` | `int` | 0-based among matching tracks. Raises if missing. |
| `MP4GetTrackType(hFile, trackId)` | `str` | e.g. `"vide"` / `"soun"`. |
| `MP4DeleteTrack(hFile, trackId)` | `None` | Control info only; call `MP4Optimize` to drop sample data. |
| `MP4AddAudioTrack(hFile, timeScale, sampleDuration, audioType=MP4_MPEG4_AUDIO_TYPE)` | `int` | New audio track id. |
| `MP4AddVideoTrack(hFile, timeScale, sampleDuration, width, height, videoType=MP4_MPEG4_VIDEO_TYPE)` | `int` | New video track id. |
| `MP4HaveAtom(hFile, atomName)` | `bool` | Predicate; does not raise when the atom is absent. |
| `MP4GetDuration(hFile)` / `MP4GetTimeScale(hFile)` | `int` | Movie duration (timescale units) and ticks/sec. |
| `MP4SetTimeScale(hFile, value)` | `None` | |
| `MP4GetTrackDuration` / `MP4GetTrackTimeScale` / `MP4SetTrackTimeScale` | `int` / `None` | Track timeline. |
| `MP4GetTrackVideoWidth` / `MP4GetTrackVideoHeight` | `int` | Pixels. |
| `MP4GetTrackLanguage` / `MP4SetTrackLanguage` | `str` / `None` | ISO-639-2/T, 3 letters. |
| `MP4GetTrackName` / `MP4SetTrackName` | `str` / `None` | `udta.name`; getter copies and `MP4Free`s. |
| `MP4ChangeMovieTimeScale(hFile, value)` | `None` | Recalculate `moov.mvhd` fields for a new movie timescale. |
| `MP4GetIntegerProperty` / `MP4SetIntegerProperty` | `int` / `None` | Generic atom property, e.g. `moov.mvhd.timescale`. |
| `MP4GetFloatProperty` / `MP4SetFloatProperty` | `float` / `None` | e.g. `moov.mvhd.rate`. |
| `MP4GetStringProperty` / `MP4SetStringProperty` | `str` / `None` | e.g. `ftyp.majorBrand`. |
| `MP4GetBytesProperty` / `MP4SetBytesProperty` | `bytes` / `None` | Getter copies and `MP4Free`s. |
| `MP4HaveTrackAtom(hFile, trackId, atomName)` | `bool` | Predicate; does not raise when the atom is absent. |
| `MP4GetTrackBitRate` / `MP4GetTrackVideoFrameRate` / `MP4GetTrackAudioChannels` | `int` / `float` / `int` | Inspection. Audio channels raise if the track has none. |
| `MP4GetTrackMediaDataName` / `MP4GetTrackFixedSampleDuration` | `str` / `int` | Sample-entry four-cc; fixed duration or `MP4_INVALID_DURATION`. |
| `MP4GetTrackESConfiguration` / `MP4SetTrackESConfiguration` | `bytes` / `None` | Getter copies and `MP4Free`s. |
| `MP4GetTrackIntegerProperty` (and Float/String/Bytes + setters) | typed / `None` | Track-scoped generic properties, e.g. `tkhd.layer`. |
| `MP4WriteSample(hFile, trackId, pBytes, duration=MP4_INVALID_DURATION, renderingOffset=0, isSyncSample=True)` | `None` | `pBytes` is `bytes`. First sample id is 1. |
| `MP4ReadSample(hFile, trackId, sampleId)` | `MP4Sample` | `.data` is `bytes`. Unpacks as `(data, startTime, duration, renderingOffset, isSyncSample)`. |
| `MP4ReadSampleFromTime(hFile, trackId, when)` | `MP4Sample` | Sample containing `when` (track timescale). |
| `MP4GetSampleIdFromTime(hFile, trackId, when, wantSyncSample=False)` | `int` | Raises if none. |
| `MP4GetSampleTime` / `MP4GetSampleDuration` / `MP4GetSampleSync` | `int` / `int` / `bool` | Per-sample timing; raise on `MP4_INVALID_*` / `-1`. |
| `MP4CopySample(srcFile, srcTrackId, srcSampleId, dstFile=None, dstTrackId=MP4_INVALID_TRACK_ID, dstSampleDuration=MP4_INVALID_DURATION)` | `None` | `dstFile` None copies in the source file. |
| `MP4GetSampleSize` / `MP4GetTrackNumberOfSamples` / `MP4GetTrackMaxSampleSize` | `int` | |
| `MP4TagsAlloc()` | `MP4Tags` | Also constructible as `MP4Tags()`. Context manager calls `MP4TagsFree`. |
| `MP4TagsFetch` / `MP4TagsStore` / `MP4TagsFree` | `None` | Fetch into / store from the snapshot. `MP4TagsHasMetadata` reflects the last Fetch. |
| `MP4TagsSetName` (and other string setters) | `None` | `None` removes the tag. Read via `tags.name`, `tags.artist`, … |
| `MP4TagsSetCompilation` / `MP4TagsSetGenreType` / `MP4TagsSetTVSeason` (and other scalar setters) | `None` | Numeric iTMF fields. `None` removes. Read via `tags.compilation`, `tags.genreType`, … |
| `MP4TagsAddArtwork(tags, data, type=MP4_ART_UNDEFINED)` | `None` | `data` is `bytes`. `tags.artwork` is a list of copies (`data` + `type`). |
| `MP4GetChapters(hFile, chapterType=MP4ChapterTypeQt)` | `MP4ChapterList` | Unpacks as `(type, chapters)`. Copies and `MP4Free`s the C array. Empty if none. |
| `MP4SetChapters(hFile, chapterList, chapterType=MP4ChapterTypeQt)` | `MP4ChapterType` | `chapterList` is a sequence of `MP4Chapter` (duration in milliseconds). Nero's last chapter duration on read is "until end of movie". |
| `MP4AddChapterTextTrack` / `MP4AddChapter` / `MP4AddNeroChapter` | id / `None` | Qt text track + chapters; Nero start time is 100 ns units. |
| `MP4ConvertChapters` / `MP4DeleteChapters` | `MP4ChapterType` | `MP4ChapterTypeNone` if there was nothing to convert/delete. |

Also exported on `raw`: `MP4_INVALID_TRACK_ID`, `MP4_INVALID_SAMPLE_ID`,
`MP4_INVALID_DURATION`, `MP4_INVALID_TIMESTAMP`, `MP4_INVALID_EDIT_ID`,
`MP4_CREATE_64BIT_DATA`, `MP4_CREATE_64BIT_TIME`, track type strings
(`MP4_VIDEO_TRACK_TYPE`, …), `MP4FileHandle` (`close_flags`), `MP4LogLevel`,
`MP4Sample`, `MP4Tags`, `MP4TagArtwork` (bytes + type, not a raw `void*`),
`MP4TagTrack`, `MP4TagDisk`, `MP4TagArtworkType_e`, `MP4Chapter`, `MP4ChapterType`,
`MP4V2_CHAPTER_TITLE_MAX`.

`MP4TrackId`, `MP4SampleId`, `MP4Timestamp`, `MP4Duration`, and `MP4EditId` are
`int` aliases (not `typing.NewType`).

**Not bound (Alpha, out of scope):** iTMF generic atoms, MPEG-4 profile-level
setters, hint/streaming, custom I/O callbacks (`MP4*Callbacks`), deprecated
`MP4ReadProvider`, encrypted-sample helpers. The high-level path is inspect /
tag / sample I/O / chapters / optimize.
