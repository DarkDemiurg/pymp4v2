## Installation

Wheels (manylinux, Windows, macOS) and an sdist are attached to
[GitHub Releases](https://github.com/DarkDemiurg/pymp4v2/releases) for `v*`
tags. They are not published to PyPI.

Download a wheel for your platform from the latest release, then:

```bash
pip install pymp4v2-*.whl
```

To build from git:

```bash
pip install git+https://github.com/DarkDemiurg/pymp4v2.git
```

### From source

```bash
git clone https://github.com/DarkDemiurg/pymp4v2.git
cd pymp4v2
pip install .
```

The build uses **scikit-build-core** and CMake. [mp4v2 v2.1.3](https://github.com/enzo1982/mp4v2/releases/tag/v2.1.3) is downloaded by FetchContent from the official release tarball (SHA256-pinned) and statically linked. The first configure needs network unless that tarball is already in the CMake FetchContent cache (`build/` when using the default `build-dir`).

### Development

```bash
pip install -e ".[dev]"
pytest tests/
```

## Build requirements

- Python 3.9+
- C++17 compiler (GCC, Clang, or MSVC 2022)
- CMake 3.28+ (installed automatically via pip if missing)
- Ninja (optional; installed automatically via pip)

### Windows

1. Install Visual Studio 2022 with the C++ workload
2. Install Python 3.9+
3. From a Developer Command Prompt:

```bash
pip install .
```

### Linux

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential python3-dev

pip install .
```

### macOS

Install Xcode Command Line Tools, then:

```bash
xcode-select --install
pip install .
```
