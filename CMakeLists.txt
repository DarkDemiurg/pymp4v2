cmake_minimum_required(VERSION 3.14)

set(PROJECT_NAME pymp4v2)
set(PROJECT_VERSION 0.1.4)

project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# Создаем список аргументов CMake
set(MP4V2_CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DBUILD_SHARED=OFF
    -DBUILD_UTILS=OFF
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    -DCMAKE_CXX_STANDARD=17
    -DCMAKE_CXX_STANDARD_REQUIRED=ON
    -DCMAKE_CXX_EXTENSIONS=OFF
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5
)

if(UNIX)
    list(APPEND MP4V2_CMAKE_ARGS -DCMAKE_CXX_FLAGS=-fPIC)
endif()

# Download and build MP4v2
ExternalProject_Add(mp4v2_external
    GIT_REPOSITORY ${MP4V2_URL}
    GIT_TAG ${MP4V2_TAG}
    PREFIX "${CMAKE_BINARY_DIR}/mp4v2"
    INSTALL_DIR "${CMAKE_BINARY_DIR}/mp4v2_install"
    CMAKE_ARGS ${MP4V2_CMAKE_ARGS}
    BUILD_ALWAYS OFF
    UPDATE_COMMAND ""
)

ExternalProject_Get_Property(mp4v2_external install_dir)
set(MP4V2_INCLUDE_DIR "${install_dir}/include")
set(MP4V2_LIBRARY "${install_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}mp4v2${CMAKE_STATIC_LIBRARY_SUFFIX}") # или .lib для Windows

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
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

if(WIN32)
    if(DEFINED pybind11_DIR AND NOT pybind11_DIR STREQUAL "" AND NOT pybind11_DIR MATCHES "NOTFOUND" AND EXISTS "${pybind11_DIR}")
        message(STATUS "Using pybind11 from: ${pybind11_DIR}")
        find_package(pybind11 REQUIRED)
    else()
        message(STATUS "pybind11_DIR not set or invalid, trying to find pybind11 automatically...")

        execute_process(
            COMMAND ${Python3_EXECUTABLE} -c "try:\n    import pybind11\n    import os\n    print(os.path.dirname(pybind11.__file__))\nexcept:\n    exit(1)"
            OUTPUT_VARIABLE PYTHON_PYBIND11_PATH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE pybind11_find_result
        )

        if(pybind11_find_result EQUAL 0)
            set(PYBIND11_CMAKE_PATH "${PYTHON_PYBIND11_PATH}/share/cmake/pybind11")
            if(EXISTS "${PYBIND11_CMAKE_PATH}")
                set(pybind11_DIR "${PYBIND11_CMAKE_PATH}" CACHE PATH "Path to pybind11 CMake files" FORCE)
            else()
                set(PYBIND11_CMAKE_PATH "${PYTHON_PYBIND11_PATH}/../share/cmake/pybind11")
                get_filename_component(PYBIND11_CMAKE_PATH "${PYBIND11_CMAKE_PATH}" ABSOLUTE)

                if(EXISTS "${PYBIND11_CMAKE_PATH}")
                    set(pybind11_DIR "${PYBIND11_CMAKE_PATH}" CACHE PATH "Path to pybind11 CMake files" FORCE)
                else()
                    set(pybind11_DIR "${PYTHON_PYBIND11_PATH}" CACHE PATH "Path to pybind11" FORCE)
                endif()
            endif()

            find_package(pybind11 REQUIRED)
        endif()
    endif()

else()
    set(PYBIND11_FINDPYTHON ON)
    find_package(pybind11 CONFIG REQUIRED)
endif()

# make python module
pybind11_add_module(${MODULE_NAME}
    src/main.cpp
    src/mp4file.cpp
    src/raw.cpp
    src/mp4_file_handle_wrapper.cpp
)

# Set output directory for the library
if(CMAKE_CONFIGURATION_TYPES)
    # For multi-config generators (like MSVC)
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
        set_target_properties(${MODULE_NAME} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}
        )
    endforeach()
else()
    # For single-config generators
    set_target_properties(${MODULE_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}
    )
endif()

# Set output directory for the library
set_target_properties(${MODULE_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}
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

if(WIN32)
    message(STATUS "Building on Windows")
    target_compile_definitions(${MODULE_NAME} PRIVATE MP4V2_USE_STATIC_LIB=1)
else()
    message(STATUS "Building on Unix-like system")
endif()

# add package info
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/package_info.py.in
    ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}/package_info.py
    @ONLY
)

# stub-files generation
find_program(PYBIND11_STUBGEN pybind11-stubgen)
if(PYBIND11_STUBGEN)
	if(WIN32)
		add_custom_target(generate_stubs ALL
			COMMAND ${CMAKE_COMMAND} -E env
			"PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME};$ENV{PYTHONPATH}"
			${PYBIND11_STUBGEN} ${MODULE_NAME}
			-o ${CMAKE_CURRENT_BINARY_DIR}
			COMMENT "Generating Python stub files"
			DEPENDS ${MODULE_NAME}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		)
	else()
		add_custom_target(generate_stubs ALL
			COMMAND ${CMAKE_COMMAND} -E env
			PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}:$ENV{PYTHONPATH}
			${PYBIND11_STUBGEN} ${MODULE_NAME}
			-o ${CMAKE_CURRENT_BINARY_DIR}
			COMMENT "Generating Python stub files"
			DEPENDS ${MODULE_NAME}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		)	
	endif()

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}-stubs/${MODULE_NAME}.pyi
        DESTINATION lib/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages/pymp4v2
    )
else()
    message(WARNING "pybind11-stubgen not found")
endif()

# Installing
install(TARGETS ${MODULE_NAME}
    LIBRARY DESTINATION lib/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages
)

# Make sure cleanup runs after generate_stubs
# add_custom_command(TARGET generate_stubs
#     COMMAND ${CMAKE_COMMAND} -DCMAKE_BINARY_DIR=${CMAKE_BINARY_DIR} -P ${CMAKE_SOURCE_DIR}/cmake/PostBuildCleanup.cmake
#     COMMENT "Running post-build cleanup script"
# )
