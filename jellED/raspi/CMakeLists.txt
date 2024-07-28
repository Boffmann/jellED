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

add_subdirectory(src)
