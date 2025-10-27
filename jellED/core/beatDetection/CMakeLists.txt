cmake_minimum_required(VERSION 3.16.0)

set(target BeatDetection)

project(${target})

add_compile_options(-std=c++17)

set(include_path
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "$ENV{MC_LIB_PATH}"
)

set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(sources
  ${source_path}/bandpassFilter.cpp
  ${source_path}/envelopeDetector.cpp
  ${source_path}/ringbuffer.cpp
  ${source_path}/peakdetection.cpp
  ${source_path}/beatdetection.cpp
)

add_library(${target} ${sources})

target_include_directories(${target}
  PUBLIC
  ${include_path}
)
