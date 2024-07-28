cmake_minimum_required(VERSION 3.16.0)

set(target BeatDetection)

project(${target})

add_compile_options(-std=c++17)

set(include_path
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(sources
  ${source_path}/bandpassFilter.cpp
  ${source_path}/envelopeDetector.cpp
  ${source_path}/ringbuffer.cpp
  ${source_path}/peakdetection.cpp
  ${source_path}/beatdetection.cpp
)

# Option to build tests (disabled by default)
option(BUILD_BEATDETECTION_TESTS "Build BeatDetection tests" OFF)

# Only build tests if explicitly requested
if(BUILD_BEATDETECTION_TESTS)
    include(FetchContent)
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    FetchContent_MakeAvailable(googletest)
    enable_testing()
    add_subdirectory(test)
endif()

add_library(${target} ${sources})

target_include_directories(${target}
  PRIVATE
  ${include_path}
)
