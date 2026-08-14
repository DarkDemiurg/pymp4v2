[![CI](https://github.com/DarkDemiurg/pymp4v2/actions/workflows/ci.yml/badge.svg)](https://github.com/DarkDemiurg/pymp4v2/actions/workflows/ci.yml)
[![Wheels](https://github.com/DarkDemiurg/pymp4v2/actions/workflows/publish.yml/badge.svg)](https://github.com/DarkDemiurg/pymp4v2/actions/workflows/publish.yml)
[![codecov](https://codecov.io/gh/DarkDemiurg/pymp4v2/graph/badge.svg?token=O01EZO3V0Q)](https://codecov.io/gh/DarkDemiurg/pymp4v2)
[![Tests](https://img.shields.io/github/actions/workflow/status/DarkDemiurg/pymp4v2/ci.yml?label=tests)](https://github.com/DarkDemiurg/pymp4v2/actions)
[![Python](https://img.shields.io/badge/python-3.9%20|%203.10%20|%203.11%20|%203.12%20|%203.13-blue.svg)](https://www.python.org/)
[![Platforms](https://img.shields.io/badge/platform-windows%20|%20linux%20|%20macos-lightgrey.svg)](https://github.com/DarkDemiurg/pymp4v2)
[![GitHub License](https://img.shields.io/github/license/DarkDemiurg/pymp4v2)](https://github.com/DarkDemiurg/pymp4v2/blob/master/LICENSE)
[![GitHub Release](https://img.shields.io/github/v/release/DarkDemiurg/pymp4v2)](https://github.com/DarkDemiurg/pymp4v2/releases)
[![Dependencies](https://img.shields.io/librariesio/github/DarkDemiurg/pymp4v2)](https://libraries.io/github/DarkDemiurg/pymp4v2)

# pymp4v2 - Python Bindings for [mp4v2](https://github.com/enzo1982/mp4v2)

**Alpha.** Python bindings for a **subset** of the [mp4v2](https://github.com/enzo1982/mp4v2) C API (v2.1.3). This is not a complete wrap of mp4v2: hint/streaming, generic iTMF, and most profile-level APIs are out of scope for Alpha.

## What is bound

- `pymp4v2.MP4File` — open with `mode` `"r"` / `"w"` / `"a"` (`"r+"`), context manager, `.tracks` / `Track`, `.tags` / `Tags`, `duration` / `timescale`, `get_info()`, `optimize()`, `save()`, `close()`
- `pymp4v2.raw` — file I/O (`MP4CreateEx` for ftyp brands / iods; no I/O callbacks), tracks, file/track properties (named + generic get/set), sample read/write/copy (`bytes`, including lookup by time), iTMF tags (string + numeric setters, artwork as `bytes`), chapters (`MP4GetChapters` / `MP4SetChapters` / Qt + Nero)
- `pymp4v2.MP4Error` — raised on C failures (`false` / `MP4_INVALID_*`); subclass of `RuntimeError`
- PEP 561 type stubs (`.pyi` + `py.typed`) ship in the wheel

Not bound (Alpha, out of scope): iTMF generic atoms, hint/streaming, custom I/O callbacks, `MP4ReadProvider`, MPEG-4 profile-level setters.

## Requirements

### Build Dependencies

- Python 3.9+
- C++17 compatible compiler
- CMake 3.28+ (pulled in by the build backend if missing)
- Ninja (optional; pulled in by the build backend)

The extension is built with **scikit-build-core**. mp4v2 v2.1.3 is fetched by CMake FetchContent and statically linked.

### Python Dependencies

- scikit-build-core >= 0.10
- pybind11 >= 2.12

## Installation

See the [Installation](https://github.com/DarkDemiurg/pymp4v2/blob/master/docs/installation.md) file.

## Usage

See the [Usage](https://github.com/DarkDemiurg/pymp4v2/blob/master/docs/usage.md) file.

Scripts in [`examples/`](examples/) (`dump_info.py`, `retag.py`, `extract_samples.py`) exercise the same APIs against a real file.

## API Reference

See the [API](https://github.com/DarkDemiurg/pymp4v2/blob/master/docs/api.md) file.

## Running Tests

Tests generate a tiny MP4 with `MP4Create`; they do not need `tests/sample.mp4`.

```bash
pip install -e .[dev]
pytest tests/
```

## Contributing

See the [CONTRIBUTING.md](https://github.com/DarkDemiurg/pymp4v2/blob/master/CONTRIBUTING.md) file.

## License

The pymp4v2 bindings are [MIT](https://github.com/DarkDemiurg/pymp4v2/blob/master/LICENSE).

The native extension **statically links** [mp4v2](https://github.com/enzo1982/mp4v2) v2.1.3, which is [MPL 1.1](https://github.com/DarkDemiurg/pymp4v2/blob/master/licenses/mp4v2/COPYING). See [NOTICE](https://github.com/DarkDemiurg/pymp4v2/blob/master/NOTICE) for the pinned tarball URL and SHA256. The corresponding mp4v2 source is the official v2.1.3 release.

## Acknowledgments

- [mp4v2](https://github.com/enzo1982/mp4v2) library authors for the underlying C++ library
- Pybind11 team for the excellent binding infrastructure

## Support

If you encounter any issues or have questions, please file an issue on the GitHub issue tracker.

## Changelog

See the [CHANGELOG.md](https://github.com/DarkDemiurg/pymp4v2/blob/master/CHANGELOG.md) file for a history of changes to the project.
