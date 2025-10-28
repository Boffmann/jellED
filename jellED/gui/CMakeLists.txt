cmake_minimum_required(VERSION 3.10)
project(jellEDOszilloscope)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable automatic MOC (Meta-Object Compiler) for Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt5 packages
find_package(Qt5 COMPONENTS Widgets REQUIRED)

LINK_DIRECTORIES(
  "/usr/local/lib"
  "/opt/homebrew/lib"
)

add_subdirectory($ENV{MC_LIB_PATH}/sound/raspi ${CMAKE_BINARY_DIR}/sound/raspi)

set(include_path
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../core/beatDetection"
    "$ENV{MC_LIB_PATH}"
    "/usr/local/include"
    "/opt/homebrew/include"
)

# Source files
set(SOURCES
    src/main.cpp
    src/AudioDisplay.cpp
    src/WaveformWidget.cpp
    ../core/beatDetection/src/bandpassFilter.cpp
    ../core/beatDetection/src/ringbuffer.cpp
)

# Header files
set(HEADERS
    include/AudioDisplay.h
    include/WaveformWidget.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME}
  PRIVATE
  ${include_path}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    PRIVATE
    Qt5::Widgets
    soundio
    raspi_sound
)

# Platform-specific settings
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
    )
endif()
