#include "../beatdetection.h"
#include "utils/TestPlatformUtils.h"

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>

class BeatDetectionTest : public testing::Test {
  protected:
    std::vector<double> samples;
    std::vector<double> expected_result;
    static constexpr uint32_t SAMPLE_RATE = 44100;
    static constexpr uint32_t ENVELOPE_DOWNSAMPLE_RATIO = 8;

    BeatDetectionTest() {
        read_resources();
    }

    ~BeatDetectionTest() {

    }

    void read_resources() {
        // Get the path of the current source file
        std::string source_path = __FILE__;
        std::filesystem::path source_dir =
            std::filesystem::path(source_path).parent_path();

        std::filesystem::path unfiltered_test_values =
            source_dir / "resources/beatDetection/unfiltered_test_values.txt";
        std::ifstream unfilteredFile(unfiltered_test_values);

        if (!unfilteredFile.is_open()) {
            std::cerr << "Failed to open the enveloped file: " << std::strerror(errno)
                        << std::endl;
            return;
        }

        std::filesystem::path peaks_file_path =
            source_dir / "resources/beatDetection/peak_values.txt";
        std::ifstream peaksFile(peaks_file_path);

        if (!peaksFile.is_open()) {
            std::cerr << "Failed to open the filtered file: " << std::strerror(errno)
                        << std::endl;
            return;
        }

        std::string line;
        while (getline(unfilteredFile, line)) {
            double sample = std::stod(line);
            samples.push_back(sample);
        }
        while (getline(peaksFile, line)) {
            double sample = std::stod(line);
            expected_result.push_back(sample);
        }
    }
};

TEST_F(BeatDetectionTest, TestApplyBeatDetection) {
  ASSERT_GT(samples.size(), 0);
  ASSERT_GT(expected_result.size(), 0);
  
  TestPlatformUtils utils;
  BeatDetector beatDetector(utils, SAMPLE_RATE);

  
  std::vector<double> actual_result;

  for (int envelope_sample_index = 0; envelope_sample_index < samples.size(); ++envelope_sample_index) {
  //for (int envelope_sample_index = 0; envelope_sample_index < 10; ++envelope_sample_index) {
    double sample = samples.at(envelope_sample_index);
    if (beatDetector.is_beat(sample)) {
        actual_result.push_back(envelope_sample_index);
    }
  }

  // We need to scale the actual_result to the downsampled rate from the test file
  std::for_each(actual_result.begin(), actual_result.end(), [&](double &sample) {
    sample = std::floor(sample / ENVELOPE_DOWNSAMPLE_RATIO);
  });

  ASSERT_EQ(actual_result.size(), expected_result.size());

  for (int vector_index = 0; vector_index < expected_result.size();
       vector_index++) {
    double expected_sample = expected_result.at(vector_index);
    double actual_sample = actual_result.at(vector_index);

    ASSERT_NEAR(actual_sample, expected_sample, 0.000001)
        << "Expected: " << expected_sample << " Actual: " << actual_sample
        << " Index: " << vector_index;
  }
}
