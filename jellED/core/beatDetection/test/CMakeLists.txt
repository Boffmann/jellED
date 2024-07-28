include(GoogleTest)
# Ringbuffer Tests Executable
add_executable(ringbuffer_test
    ringbuffer_test.cpp
)

target_link_libraries(ringbuffer_test
    GTest::gtest_main
    BeatDetection
)

gtest_discover_tests(ringbuffer_test)

# Bandpass Filter Tests Executable
add_executable(bandpass_test
    bandpass_test.cpp
)

target_link_libraries(bandpass_test
    GTest::gtest_main
    BeatDetection
)

gtest_discover_tests(bandpass_test)

# Envelope Filter Tests Executable
add_executable(envelope_test
    envelope_test.cpp
)

target_link_libraries(envelope_test
    GTest::gtest_main
    BeatDetection
)

gtest_discover_tests(envelope_test)

# Peak Detection Tests Executable
add_executable(peakdetection_test
    peakdetection_test.cpp
)

target_link_libraries(peakdetection_test
    GTest::gtest_main
    BeatDetection
)

gtest_discover_tests(peakdetection_test)
