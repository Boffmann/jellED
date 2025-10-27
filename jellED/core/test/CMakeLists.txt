include(GoogleTest)

add_library(TestUtils 
    utils/TestPlatformUtils.cpp
)

target_include_directories(TestUtils
    PRIVATE
    "$ENV{MC_LIB_PATH}"
)

# Pattern Tests Executable
add_executable(pattern_test
    patternEngine/rainbowpattern_test.cpp
    patternEngine/patternEngine_test.cpp
)

# BeatDetection Tests Executable
add_executable(beatDetection_test
    beatDetection/bandpass_test.cpp
    beatDetection/beatdetection_test.cpp
    beatDetection/envelope_test.cpp
    beatDetection/peakdetection_test.cpp
    beatDetection/ringbuffer_test.cpp
)

target_link_libraries(pattern_test
    GTest::gtest_main
    PatternEngine
    TestUtils
)

target_link_libraries(beatDetection_test
    GTest::gtest_main
    BeatDetection
    TestUtils
)

gtest_discover_tests(pattern_test)
gtest_discover_tests(beatDetection_test)