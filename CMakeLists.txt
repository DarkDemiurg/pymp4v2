cmake_minimum_required(VERSION 3.14)

set(PROJECT_NAME pymp4v2)
set(PROJECT_VERSION 0.1.0)

project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

set(MODULE_NAME pymp4v2)
set(PROJECT_AUTHOR "Dmitriy Efimov <daefimov@gmail.com>")
set(PROJECT_DESCRIPTION "Python bindings for MP4v2 library")

# Set default build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Set default build type: RelWithDebInfo")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Тип сборки" FORCE)
  
  # Set possible build types for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

####################### Building mp4v2

set(MP4V2_URL "https://github.com/enzo1982/mp4v2.git")
set(MP4V2_TAG "v2.1.3")

include(ExternalProject)

# Download and build MP4v2
ExternalProject_Add(mp4v2_external
    GIT_REPOSITORY ${MP4V2_URL}
    GIT_TAG ${MP4V2_TAG}
    PREFIX "${CMAKE_BINARY_DIR}/mp4v2"
    INSTALL_DIR "${CMAKE_BINARY_DIR}/mp4v2_install"
    CMAKE_ARGS 
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_BUILD_TYPE=Release
        -DBUILD_SHARED=OFF
        -DBUILD_UTILS=OFF
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        -DCMAKE_CXX_FLAGS=-fPIC
    BUILD_ALWAYS OFF
    UPDATE_COMMAND ""
)

ExternalProject_Get_Property(mp4v2_external install_dir)
set(MP4V2_INCLUDE_DIR "${install_dir}/include")
set(MP4V2_LIBRARY "${install_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}mp4v2${CMAKE_STATIC_LIBRARY_SUFFIX}")  # или .lib для Windows

if(NOT EXISTS "${MP4V2_INCLUDE_DIR}")
    # message(WARNING "MP4V2 include directory does not exist, creating: ${MP4V2_INCLUDE_DIR}")
    file(MAKE_DIRECTORY "${MP4V2_INCLUDE_DIR}")
endif()

add_library(mp4v2 STATIC IMPORTED)
set_target_properties(mp4v2 PROPERTIES
    IMPORTED_LOCATION "${MP4V2_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${MP4V2_INCLUDE_DIR}"
)

add_dependencies(mp4v2 mp4v2_external)

####################### Finish building mp4v2

# Find Python and pybind11
set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

# make python module
pybind11_add_module(${MODULE_NAME} 
    src/main.cpp
    src/mp4file.cpp
    src/raw.cpp
    src/mp4_file_handle_wrapper.cpp
)

# include directories setup
target_include_directories(${MODULE_NAME} PRIVATE
    ${MP4V2_INCLUDE_DIR}
    ${pybind11_INCLUDE_DIRS}
    include
)

# link setup
target_link_libraries(${MODULE_NAME} PRIVATE
    mp4v2
    ${pybind11_LIBRARIES}
    ${Python_LIBRARIES}
)

# add package info
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/package_info.py.in
    ${CMAKE_CURRENT_BINARY_DIR}/package_info.py
    @ONLY
)

# copy package_info.py into packet
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/package_info.py.in
    ${CMAKE_CURRENT_BINARY_DIR}/pymp4v2/package_info.py
    @ONLY
)

# stub-files generation
find_program(PYBIND11_STUBGEN pybind11-stubgen)
if(PYBIND11_STUBGEN)
    # Create a custom target for stub generation that runs after the module is built
    add_custom_target(generate_stubs ALL
        COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}:$ENV{PYTHONPATH} ${PYBIND11_STUBGEN} ${MODULE_NAME} -o ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating Python stub files"
        DEPENDS ${MODULE_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    
    # stub-files installation
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}-stubs/${MODULE_NAME}.pyi
            DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages)
else()
    message(WARNING "pybind11-stubgen not found")
endif()

# make python package
# file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/pymp4v2)
# file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/pymp4v2/core)

# # creation of __init__.py files
# file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pymp4v2/__init__.py "
# from .core.module import *
# from .package_info import *
# ")

# file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pymp4v2/core/__init__.py "
# from .module import *
# ")

# Installing
install(TARGETS ${MODULE_NAME}
    LIBRARY DESTINATION lib/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages
)
