## Usage

pymp4v2 is **Alpha** and exposes only a subset of mp4v2. Use `MP4File` for a small
Pythonic wrapper, or `pymp4v2.raw` for the bound C functions.

### MP4File

```python
import pymp4v2
import pymp4v2.raw as raw

# Read an existing file (`mode="r"` is the default)
with pymp4v2.MP4File("video.mp4") as mp4:
    print(mp4.filename, mp4.mode)
    print(mp4.get_track_count())
    print(mp4.info)
    for track in mp4.tracks:
        print(track.id, track.type, track.duration, track.sample_count,
              track.bitrate, track.frame_rate, track.audio_channels)
        if track.sample_count:
            sample = track.read_sample(0)  # 0-based; raw.MP4ReadSample is 1-based
            print(sample.data[:16], sample.duration)

# Create a new empty MP4 (truncates if the path exists)
with pymp4v2.MP4File("output.mp4", "w") as mp4:
    mp4.tags.name = "Title"
    mp4.tags.artist = "Artist"
    mp4.tags.add_artwork(open("cover.jpg", "rb").read(), raw.MP4_ART_JPEG)
    mp4.save()  # close+reopen so pending writes are on disk and the handle stays open

# Modify an existing file (`"a"` or `"r+"`)
with pymp4v2.MP4File("video.mp4", "a") as mp4:
    mp4.tags.album = "Album"
    mp4.optimize()  # persist, rewrite layout, reopen
```

`save()` is not a separate mp4v2 flush: the library writes on `MP4Close`. For
writable files, `save()` closes and reopens with `MP4Modify`. Read-only `save()`
is a no-op. Unknown modes and failed opens raise `MP4Error` (a `RuntimeError`
subclass) and include the filename.

### raw module

```python
import pymp4v2.raw as mp4

with mp4.MP4Read("video.mp4") as handle:
    filename = mp4.MP4GetFilename(handle)
    print(f"File: {filename}")

    info = mp4.MP4Info(handle)  # str or None; memory is freed by the wrapper
    print(f"Info: {info}")

    n = mp4.MP4GetNumberOfTracks(handle)
    for i in range(n):
        tid = mp4.MP4FindTrackId(handle, i)
        print(tid, mp4.MP4GetTrackType(handle, tid), mp4.MP4GetTrackDuration(handle, tid))

    ok = mp4.MP4Dump(handle)  # prints to stdout; returns True or raises MP4Error
    print(f"Dump succeeded: {ok}")

with mp4.MP4Create("output.mp4") as handle:
    tid = mp4.MP4AddAudioTrack(handle, 44100, 1024)
    mp4.MP4WriteSample(handle, tid, b"\x00\x01", duration=1024)
    sample = mp4.MP4ReadSample(handle, tid, 1)  # sample ids start at 1
    print(sample.data, sample.duration)
    sid = mp4.MP4GetSampleIdFromTime(handle, tid, 0)
    same = mp4.MP4ReadSampleFromTime(handle, tid, 0)
    print(sid, same.data, mp4.MP4GetSampleDuration(handle, tid, sid))

    with mp4.MP4TagsAlloc() as tags:
        mp4.MP4TagsSetName(tags, "Title")
        mp4.MP4TagsAddArtwork(tags, open("cover.jpg", "rb").read(), mp4.MP4_ART_JPEG)
        mp4.MP4TagsStore(tags, handle)

with mp4.MP4CreateEx("branded.mp4", majorBrand="iso5", compatibleBrands=["iso5", "iso6"]) as handle:
    print(mp4.MP4GetStringProperty(handle, "ftyp.majorBrand"))

with mp4.MP4Modify("video.mp4") as handle:
    mp4.MP4SetChapters(handle, [mp4.MP4Chapter(1000, "Intro"), mp4.MP4Chapter(2000, "Next")],
                       mp4.MP4ChapterTypeNero)
    kind, chapters = mp4.MP4GetChapters(handle, mp4.MP4ChapterTypeNero)

summary = mp4.MP4FileInfo("video.mp4")  # no handle required
mp4.MP4Optimize("video.mp4", "video.opt.mp4")
```

C names are kept. Sample payloads and artwork are `bytes`. C failures
(`false` / `MP4_INVALID_*`) raise `pymp4v2.MP4Error`, except predicates such as
`MP4HaveAtom`. `MP4TrackId` and the other id aliases are `int` (not
`typing.NewType`). Set `handle.close_flags` before leaving a `with` block if
`__exit__` should pass a non-zero `MP4Close` bitmask.

### Examples

Scripts in `examples/` use the live `MP4File` API (and `raw.MP4Dump` for atom
listings). They are covered by tests against generated fixtures.

```bash
python examples/dump_info.py video.mp4
python examples/dump_info.py video.mp4 --atoms   # MP4Dump to stdout
python examples/retag.py video.mp4 --name "Title" --artist "Artist" --optimize
python examples/retag.py video.mp4 -o tagged.mp4 --album "Album" --artwork cover.jpg
python examples/extract_samples.py video.mp4 --outdir ./samples
python examples/extract_samples.py video.mp4 --track 0 --concat --outdir ./samples
```

### Error Handling

```python
import pymp4v2

try:
    with pymp4v2.MP4File("nonexistent.mp4") as mp4file:
        pass
except pymp4v2.MP4Error as e:
    print(f"Error opening file: {e}")
```
