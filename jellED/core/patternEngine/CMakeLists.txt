cmake_minimum_required(VERSION 3.16.0)

set(target PatternEngine)

project(${target})

add_compile_options(-std=c++17)

set(include_path
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/config"
    "${CMAKE_CURRENT_SOURCE_DIR}/pattern"
    "$ENV{MC_LIB_PATH}"
)

set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(config_source_path "${CMAKE_CURRENT_SOURCE_DIR}/config/src")
set(pattern_source_path "${CMAKE_CURRENT_SOURCE_DIR}/pattern/src")

set(sources
  ${source_path}/patternEngine.cpp
  ${pattern_source_path}/pattern.cpp
  ${pattern_source_path}/patternBlueprint.cpp
  ${pattern_source_path}/rainbowPattern.cpp
)

add_library(${target} ${sources})

target_include_directories(${target}
  PUBLIC
  ${include_path}
)
