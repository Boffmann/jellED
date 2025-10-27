cmake_minimum_required(VERSION 3.16.0)

set(META_PROJECT_NAME        "JellED Raspi Version")
set(META_PROJECT_DESCRIPTION "JellED Raspi Vrsion")
set(META_AUTHOR_DOMAIN       "TODO")
set(META_AUTHOR_MAINTAINER   "hendrik")
set(META_VERSION_MAJOR       "1")
set(META_VERSION_MINOR       "0")
set(META_VERSION_PATCH       "0")
set(META_VERSION_REVISION    "${GIT_REV}")
set(META_VERSION             "${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}")
set(META_NAME_VERSION        "${META_PROJECT_NAME} v${META_VERSION} (${META_VERSION_REVISION})")
set(META_CMAKE_INIT_SHA      "${GIT_REV}")
set(PROJECT_ROOT_DIR_PATH    "${CMAKE_CURRENT_SOURCE_DIR}")

# Project options
add_compile_options(-std=c++17)
# add_compile_options(-Wall -Wextra -pedantic -Werror)

# Add memory sanitizer for debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address -fsanitize=leak -g)
    add_link_options(-fsanitize=address -fsanitize=leak)
endif()

#
# Declare project
#
project(${META_PROJECT_NAME})

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# Create version file
file(WRITE "${PROJECT_BINARY_DIR}/VERSION" "${META_NAME_VERSION}")

# Find libgpiod using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIODCXX REQUIRED libgpiodcxx)

LINK_DIRECTORIES(
  "/usr/local/lib"
  "/opt/homebrew/lib"
)

set(target jellEDRaspi)

message(PROJECT_SOURCE_DIR="${INCLUDE_DIR_PATH}")

# Fetch AudioFile dependency
include(FetchContent)
FetchContent_Declare(
  AudioFile
  GIT_REPOSITORY https://github.com/adamstark/AudioFile.git
  GIT_TAG master  # Using master branch for now
)
FetchContent_MakeAvailable(AudioFile)


# Disable BeatDetection tests for Raspberry Pi build
set(BUILD_BEATDETECTION_TESTS OFF CACHE BOOL "Build BeatDetection tests" FORCE)

# Add core libraries as subdirectories
# This automatically builds all core libraries and makes them available as targets
add_subdirectory(${PROJECT_ROOT_DIR_PATH}/../core/beatDetection ${CMAKE_BINARY_DIR}/core/beatDetection)
add_subdirectory(${PROJECT_ROOT_DIR_PATH}/../core/patternEngine ${CMAKE_BINARY_DIR}/core/patternEngine)
add_subdirectory($ENV{MC_LIB_PATH}/serial/raspi ${CMAKE_BINARY_DIR}/serial/raspi)
add_subdirectory($ENV{MC_LIB_PATH}/sound/raspi ${CMAKE_BINARY_DIR}/sound/raspi)
add_subdirectory($ENV{MC_LIB_PATH}/pushButton ${CMAKE_BINARY_DIR}/pushButton)

set(include_path
    "${PROJECT_ROOT_DIR_PATH}/../core"
    "${PROJECT_ROOT_DIR_PATH}/../core/beatDetection"
    "${PROJECT_ROOT_DIR_PATH}/../core/patternEngine"
    "${PROJECT_ROOT_DIR_PATH}"
    "$ENV{MC_LIB_PATH}"
    "/usr/local/include"
    "/opt/homebrew/include"
)

set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(sources
  $ENV{MC_LIB_PATH}/pUtils/raspi/src/raspicrono.cpp
  $ENV{MC_LIB_PATH}/pUtils/raspi/src/raspilogger.cpp
  $ENV{MC_LIB_PATH}/pUtils/raspi/src/raspiinputoutput.cpp
  $ENV{MC_LIB_PATH}/pUtils/raspi/src/raspiutils.cpp
  ${source_path}/jellED.cpp
)

add_executable(${target}
  ${sources}
)

target_include_directories(${target}
  PRIVATE
  ${include_path}
  ${GPIODCXX_INCLUDE_DIRS}
)

target_link_libraries(${target}
  PRIVATE
  soundio
  ${GPIODCXX_LIBRARIES}
  BeatDetection
  PatternEngine
  raspi_serial
  raspi_sound
  pushButton
)
