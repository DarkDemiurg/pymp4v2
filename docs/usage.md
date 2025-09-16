## Usage

### Basic Example

```python
import pymp4v2.raw as mp4

# Open an MP4 file for reading
with mp4.MP4Read("video.mp4") as handle:
    # Get file information
    filename = mp4.MP4GetFilename(handle)
    print(f"File: {filename}")

    # Get detailed info about the file
    info = mp4.MP4Info(handle)
    print(f"Info: {info}")

    # Dump file structure
    dump = mp4.MP4Dump(handle)
    print(dump)


# Create a new MP4 file
with mp4.MP4Create("output.mp4") as handle:
    # Add tracks and metadata here
    pass

# Modify an existing MP4 file
with mp4.MP4Modify("video.mp4") as handle:
    # Modify tracks and metadata here
    pass
```

### Working with Tracks

```python
import pymp4v2.raw as mp4

with mp4.MP4Read("video.mp4") as handle:
    # Get the number of tracks
    track_count = mp4.MP4GetNumberOfTracks(handle)
    print(f"Number of tracks: {track_count}")

    # Iterate through tracks
    for i in  range(track_count):
        track_id = mp4.MP4FindTrackId(handle, i)
        track_type = mp4.MP4GetTrackType(handle, track_id)
        print(f"Track {i}: ID={track_id}, Type={track_type}")
```

### Error Handling

```python
import pymp4v2.raw as mp4

try:
    with mp4.MP4Read("nonexistent.mp4") as handle:
        # This will not be executed
        pass
except RuntimeError  as e:
    print(f"Error opening file: {e}")
```
