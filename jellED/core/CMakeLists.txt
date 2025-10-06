# Core libraries CMake configuration
# This file manages all core libraries for CMake-based builds

cmake_minimum_required(VERSION 3.16.0)

add_compile_options(-std=c++17)

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

# Option to build tests (disabled by default)
option(BUILD_CORE_TESTS "Build Core tests" ON)

# Only build tests if explicitly requested
if(BUILD_CORE_TESTS)
    include(FetchContent)
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    FetchContent_MakeAvailable(googletest)
    enable_testing()
    add_subdirectory(test)
endif()