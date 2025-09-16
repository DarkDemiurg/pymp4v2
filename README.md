[![CI](https://github.com/DarkDemiurg/pymp4v2/actions/workflows/ci.yml/badge.svg)](https://github.com/DarkDemiurg/pymp4v2/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/DarkDemiurg/pymp4v2/graph/badge.svg?token=O01EZO3V0Q)](https://codecov.io/gh/DarkDemiurg/pymp4v2)
[![Tests](https://img.shields.io/github/actions/workflow/status/DarkDemiurg/pymp4v2/ci.yml?label=tests)](https://github.com/DarkDemiurg/pymp4v2/actions)
[![Python](https://img.shields.io/badge/python-3.9%20|%203.10%20|%203.11%20|%203.12%20|%203.13-blue.svg)](https://www.python.org/)
[![Platforms](https://img.shields.io/badge/platform-windows%20|%20linux-lightgrey.svg)](https://github.com/DarkDemiurg/pymp4v2)
[![GitHub License](https://img.shields.io/github/license/DarkDemiurg/pymp4v2)](https://github.com/DarkDemiurg/pymp4v2/blob/master/LICENSE)
[![GitHub Release](https://img.shields.io/github/v/release/DarkDemiurg/pymp4v2)](https://github.com/DarkDemiurg/pymp4v2/releases)
[![Dependencies](https://img.shields.io/librariesio/github/DarkDemiurg/pymp4v2)](https://libraries.io/github/DarkDemiurg/pymp4v2)

# pymp4v2 - Python Bindings for [mp4v2](https://github.com/enzo1982/mp4v2) Library

Python bindings for the [mp4v2](https://github.com/enzo1982/mp4v2) library, providing a high-level interface for working with MP4 multimedia containers.

## Features

- Full access to [mp4v2](https://github.com/enzo1982/mp4v2) library functionality
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

See the [Installation](https://github.com/DarkDemiurg/pymp4v2/blob/master/docs/installation.md) file.

## Usage

See the [Usage](https://github.com/DarkDemiurg/pymp4v2/blob/master/docs/usage.md) file.

## API Reference

See the [API](https://github.com/DarkDemiurg/pymp4v2/blob/master/docs/api.md) file.

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

See the [CONTRIBUTING.md](https://github.com/DarkDemiurg/pymp4v2/blob/master/CONTRIBUTING.md) file.

## License

This project is licensed under the terms of the license included in the repository. 
See the [LICENSE](https://github.com/DarkDemiurg/pymp4v2/blob/master/LICENSE) file for details.

## Acknowledgments

- [mp4v2](https://github.com/enzo1982/mp4v2) library authors for the underlying C++ library
- Pybind11 team for the excellent binding infrastructure
- The Python community for excellent tooling and support

## Support

If you encounter any issues or have questions, please file an issue on the GitHub issue tracker.

## Changelog

See the [CHANGELOG.md](https://github.com/DarkDemiurg/pymp4v2/blob/master/CHANGELOG.md) file for a history of changes to the project.
