# Serial library CMake configuration

# Set library name
set(LIBRARY_NAME "jellED_serial")

# Create library target
add_library(${LIBRARY_NAME} INTERFACE)

# Set include directories
target_include_directories(${LIBRARY_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Set compile definitions
target_compile_definitions(${LIBRARY_NAME} INTERFACE
    JELLED_SERIAL_LIBRARY
)

# Set C++ standard
target_compile_features(${LIBRARY_NAME} INTERFACE cxx_std_17)

# Export library for parent projects
set(${LIBRARY_NAME}_FOUND TRUE PARENT_SCOPE)
set(${LIBRARY_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
