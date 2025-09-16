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