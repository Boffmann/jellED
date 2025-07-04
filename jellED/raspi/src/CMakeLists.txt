LINK_DIRECTORIES(
  "/usr/local/lib"
  "/opt/homebrew/lib"
  "${PROJECT_ROOT_DIR_PATH}/../core/beatDetection/build"
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

set(include_path
    "${PROJECT_ROOT_DIR_PATH}/../core"
    "${PROJECT_ROOT_DIR_PATH}/../core/beatDetection/include"
    "${PROJECT_ROOT_DIR_PATH}/../core/patternEngine"
    "${PROJECT_ROOT_DIR_PATH}/../core/patternEngine/config"
    "${PROJECT_ROOT_DIR_PATH}/../core/patternEngine/pattern"
    "${PROJECT_ROOT_DIR_PATH}/include"
    "/usr/local/include"
    "/opt/homebrew/include"
)

set(source_path "${CMAKE_CURRENT_SOURCE_DIR}")

set(sources
  ${source_path}/jellED.cpp
  ${source_path}/sound/usbMicro.cpp
  ${source_path}/sound/wavFile.cpp
  ${source_path}/utils/raspicrono.cpp
  ${source_path}/utils/raspilogger.cpp
  ${source_path}/utils/raspiutils.cpp
)

add_executable(${target}
  ${sources}
)

target_include_directories(${target}
  PRIVATE
  ${include_path}
)

target_link_libraries(${target}
  PRIVATE
  soundio
  BeatDetection
  AudioFile
  PatternEngine
)
