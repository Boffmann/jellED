# Core libraries CMake configuration
# This file manages all core libraries for CMake-based builds

cmake_minimum_required(VERSION 3.16.0)

# Set core libraries directory
set(CORE_LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Add BeatDetection library
if(EXISTS ${CORE_LIBS_DIR}/beatDetection/CMakeLists.txt)
    add_subdirectory(${CORE_LIBS_DIR}/beatDetection ${CMAKE_BINARY_DIR}/core/beatDetection)
endif()

# Add PatternEngine library
if(EXISTS ${CORE_LIBS_DIR}/patternEngine/CMakeLists.txt)
    add_subdirectory(${CORE_LIBS_DIR}/patternEngine ${CMAKE_BINARY_DIR}/core/patternEngine)
endif()

# TODO: Add other core libraries as they get CMake support
# if(EXISTS ${CORE_LIBS_DIR}/bluetooth/CMakeLists.txt)
#     add_subdirectory(${CORE_LIBS_DIR}/bluetooth ${CMAKE_BINARY_DIR}/core/bluetooth)
# endif()

# Export core include directories for use by parent projects
set(CORE_INCLUDE_DIRS
    ${CORE_LIBS_DIR}
    ${CORE_LIBS_DIR}/beatDetection/include
    ${CORE_LIBS_DIR}/patternEngine
    ${CORE_LIBS_DIR}/patternEngine/config
    ${CORE_LIBS_DIR}/patternEngine/pattern
    PARENT_SCOPE
) 