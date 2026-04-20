cmake_minimum_required(VERSION 3.16.0)

# JellEDDsp — general-purpose DSP primitives shared across beatDetection,
# patternEngine, and any future consumers. Currently header-only.
# Uses an INTERFACE library so consumers pick up include paths automatically
# via target_link_libraries, and so adding .cpp files later needs no
# downstream changes.

add_library(JellEDDsp INTERFACE)

target_include_directories(JellEDDsp INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(JellEDDsp INTERFACE cxx_std_17)
