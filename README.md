[![CI](https://github.com/DarkDemiurg/pymp4v2/actions/workflows/ci.yml/badge.svg)](https://github.com/DarkDemiurg/pymp4v2/actions/workflows/ci.yml)
[![Coverage](https://codecov.io/gh/DarkDemiurg/pymp4v2/branch/master/graph/badge.svg)](https://codecov.io/gh/DarkDemiurg/pymp4v2)
[![Tests](https://img.shields.io/github/actions/workflow/status/DarkDemiurg/pymp4v2/ci.yml?label=tests)](https://github.com/DarkDemiurg/pymp4v2/actions)
[![Python](https://img.shields.io/badge/python-3.9%20|%203.10%20|%203.11%20|%203.12%20|%203.13-blue.svg)](https://www.python.org/)
[![Platforms](https://img.shields.io/badge/platform-windows%20|%20linux-lightgrey.svg)](https://github.com/DarkDemiurg/pymp4v2)
[![License](https://img.shields.io/github/license/DarkDemiurg/pymp4v2.svg)](https://github.com/DarkDemiurg/pymp4v2/blob/master/LICENSE)
[![Release](https://img.shields.io/github/v/release/DarkDemiurg/pymp4v2.svg)](https://github.com/DarkDemiurg/pymp4v2/releases)
[![Dependencies](https://img.shields.io/librariesio/github/DarkDemiurg/pymp4v2)](https://libraries.io/github/DarkDemiurg/pymp4v2)

# pymp4v2 - Python Bindings for MP4v2 Library

Python bindings for the MP4v2 library, providing a high-level interface for working with MP4 multimedia containers.

## Features

- Full access to MP4v2 library functionality
- Support for reading, writing, and modifying MP4 files
- Cross-platform support (Windows and Linux)
- Pythonic API with proper error handling
- Automatic memory management
- Type annotations and stub files for better IDE support

## Requirements

### Build Dependencies

- Python 3.9+
- C++17 compatible compiler
- CMake 3.14+
- Ninja build system
- Git

### Python Dependencies

- setuptools >= 42
- wheel
- scikit-build >= 0.13
- cmake >= 3.14
- ninja
- pybind11 >= 2.6.0
- pybind11-stubgen >= 2.5.5

## Installation

### From Source

1. Clone the repository:

```bash
git clone https://github.com/DarkDemiurg/pymp4v2.git
cd pymp4v2
```

2. Install build dependencies:

```bash
pip install -e .[dev]
```

3. Build and install the package:

```bash
python setup.py build_ext --inplace
pip install -e .
```

### Development Installation

For development, you might want to install additional tools:

```bash
pip install -e .[dev]
```

## Building from Source

### Windows

1. Install Visual Studio 2022 with C++ support
2. Install Python 3.9+
3. Install CMake and add it to PATH
4. Open Developer Command Prompt for VS 2022
5. Run the build:

```bash
python setup.py build_ext
```

### Linux

Install build dependencies:

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential cmake ninja-build git python3-dev

# CentOS/RHEL
sudo yum groupinstall "Development Tools"
sudo yum install cmake ninja-build git python3-devel
```

Run the build:

```bash
python setup.py build_ext
```

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

## API Reference

The library provides direct access to the MP4v2 C API through the pymp4v2.raw module. Key functions include:

- `MP4Read(filename)` - Open an MP4 file for reading
- `MP4Create(filename)` - Create a new MP4 file
- `MP4Modify(filename)` - Open an MP4 file for modification
- `MP4Close(handle)` - Close an MP4 file handle
- `MP4GetFilename(handle)` - Get the filename associated with a handle
- `MP4Info(handle)` - Get information about the MP4 file
- `MP4Dump(handle)` - Dump the structure of the MP4 file

For a complete list of available functions, refer to the MP4v2 documentation and explore the `pymp4v2.raw` module.

## Running Tests

To run the test suite, you'll need to install the development dependencies:

```bash
pip install -e .[dev]
pytest tests/
```

Or use the built-in test runner:

```bash
python -m pytest tests/
```

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the terms of the license included in the repository. See the LICENSE file for details.

## Acknowledgments

- MP4v2 library authors for the underlying C++ library
- Pybind11 team for the excellent binding infrastructure
- The Python community for excellent tooling and support

## Support

If you encounter any issues or have questions, please file an issue on the GitHub issue tracker.

## Changelog

See the CHANGELOG.md file for a history of changes to the project.
