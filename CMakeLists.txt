cmake_minimum_required(VERSION 3.28)

# Single source of version: pyproject.toml. scikit-build-core injects
# SKBUILD_PROJECT_VERSION when building a wheel.
if(NOT SKBUILD_PROJECT_VERSION)
    file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/pyproject.toml" _ver_line
         REGEX "^version = \"[0-9]+\\.[0-9]+\\.[0-9]+\"")
    string(REGEX REPLACE "^version = \"([0-9.]+)\"" "\\1"
           SKBUILD_PROJECT_VERSION "${_ver_line}")
endif()

project(pymp4v2 VERSION "${SKBUILD_PROJECT_VERSION}" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# CMake 4 dropped policies older than 3.5; mp4v2 2.1.3 still declares 3.0.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)

set(BUILD_SHARED OFF CACHE BOOL "Build libmp4v2 as a shared library" FORCE)
set(BUILD_UTILS OFF CACHE BOOL "Build MP4v2 auxiliary tools" FORCE)

# Pin: enzo1982/mp4v2 v2.1.3 (commit f4c2f1b87446100728472d80fd1161d2f8e830b2).
# Official release tarball; SHA256 matches the GitHub release asset.
FetchContent_Declare(
    mp4v2
    URL https://github.com/enzo1982/mp4v2/releases/download/v2.1.3/mp4v2-2.1.3.tar.bz2
    URL_HASH SHA256=033185c17bf3c5fdd94020c95f8325be2e5356558e3913c3d6547a85dd61f7f1
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    EXCLUDE_FROM_ALL
    SYSTEM
)

FetchContent_MakeAvailable(mp4v2)

find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)

# Extension lives inside the Python package so PEP 561 stubs (py.typed + .pyi)
# can ship next to it. Import path stays pymp4v2 / pymp4v2.raw.
set(MODULE_NAME _pymp4v2)

pybind11_add_module(${MODULE_NAME}
    src/main.cpp
    src/mp4file.cpp
    src/raw.cpp
    src/raw_bind.cpp
    src/mp4_file_handle_wrapper.cpp
    src/mp4_tags_wrapper.cpp
)

target_include_directories(${MODULE_NAME} PRIVATE include)
target_link_libraries(${MODULE_NAME} PRIVATE mp4v2)
target_compile_definitions(${MODULE_NAME} PRIVATE "PYMP4V2_VERSION=\"${PROJECT_VERSION}\"")

install(TARGETS ${MODULE_NAME} DESTINATION pymp4v2)
