# Post-build cleanup script
message(STATUS "Running post-build cleanup...")

# Remove all temporary build directories
file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/mp4v2")
file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/mp4v2_install")

# Remove CMake-generated files and directories
file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/.cmake")
file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/CMakeFiles")
file(REMOVE "${CMAKE_BINARY_DIR}/CMakeCache.txt")
file(REMOVE "${CMAKE_BINARY_DIR}/cmake_install.cmake")
file(REMOVE "${CMAKE_BINARY_DIR}/Makefile")
file(REMOVE "${CMAKE_BINARY_DIR}/.ninja_deps")
file(REMOVE "${CMAKE_BINARY_DIR}/.ninja_log")

# Remove any other temporary files that might exist
file(REMOVE "${CMAKE_BINARY_DIR}/CTestTestfile.cmake")
file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/Testing")

message(STATUS "Cleanup complete. Final build artifacts are in: ${CMAKE_BINARY_DIR}/pymp4v2")
