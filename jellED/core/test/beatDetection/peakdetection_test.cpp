#include "include/peakdetection.h"

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>

class PeakDetectionTest : public testing::Test {
  protected:
    std::vector<double> samples;
    std::vector<double> expected_result;
    static constexpr uint32_t SAMPLE_RATE = 44100;
    static constexpr uint32_t ENVELOPE_DOWNSAMPLE_RATIO = 8;

    PeakDetectionTest() {
        read_resources();
    }

    ~PeakDetectionTest() {

    }

    void read_resources() {
        // Get the path of the current source file
        std::string source_path = __FILE__;
        std::filesystem::path source_dir =
            std::filesystem::path(source_path).parent_path();

        std::filesystem::path envelope_samples_file_path =
            source_dir / "resources/enveloped_test_values.txt";
        std::ifstream envelopedFile(envelope_samples_file_path);

        if (!envelopedFile.is_open()) {
            std::cerr << "Failed to open the enveloped file: " << std::strerror(errno)
                        << std::endl;
            return;
        }

        std::filesystem::path peaks_file_path =
            source_dir / "resources/peak_values.txt";
        std::ifstream peaksFile(peaks_file_path);

        if (!peaksFile.is_open()) {
            std::cerr << "Failed to open the filtered file: " << std::strerror(errno)
                        << std::endl;
            return;
        }

        std::string line;
        while (getline(envelopedFile, line)) {
            double sample = std::stod(line);
            samples.push_back(sample);
        }
        while (getline(peaksFile, line)) {
            double sample = std::stod(line);
            expected_result.push_back(sample);
        }
    }
};

TEST_F(PeakDetectionTest, TestApplyPeakDetection) {
  ASSERT_GT(samples.size(), 0);
  ASSERT_GT(expected_result.size(), 0);
  uint32_t scaled_sample_rate = SAMPLE_RATE / ENVELOPE_DOWNSAMPLE_RATIO;

  jellED::PeakDetector peakDetector(0.01, 0.1, 0.1, 180.0, scaled_sample_rate);

  std::vector<double> actual_result;
  double time_processed = 0.0;
  for (int envelope_sample_index = 0; envelope_sample_index < samples.size(); ++envelope_sample_index) {
    double sample = samples.at(envelope_sample_index);
    time_processed = ((double) envelope_sample_index) / scaled_sample_rate;
    if (peakDetector.is_peak(sample, time_processed)) {
        actual_result.push_back(envelope_sample_index);
    }
  }

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

TEST(PeakDetectionTestStandalone, AdaptsThresholdAfterTempoChange) {
  constexpr uint32_t SAMPLE_RATE = 200;
  constexpr double DELTA_T = 1.0 / SAMPLE_RATE;

  jellED::PeakDetector peakDetector(0.2, 1.0, 0.2, 220.0, SAMPLE_RATE);

  std::vector<double> envelope_samples = {
      0.05, 0.12, 0.22, 0.45, 0.9, 0.6, 0.35, 0.18,
  };

  envelope_samples.insert(envelope_samples.end(), 400, 0.30);

  std::vector<double> second_peak = {0.32, 0.38, 0.46, 0.55, 0.42, 0.28};
  envelope_samples.insert(envelope_samples.end(), second_peak.begin(), second_peak.end());

  double current_time = 0.0;
  std::vector<size_t> detected_peaks;
  detected_peaks.reserve(2);

  for (double sample : envelope_samples) {
    if (peakDetector.is_peak(sample, current_time)) {
      detected_peaks.push_back(static_cast<size_t>(std::round(current_time * SAMPLE_RATE)));
    }
    current_time += DELTA_T;
  }

  ASSERT_EQ(detected_peaks.size(), 2u);
  EXPECT_LT(detected_peaks.front(), 20u);
  EXPECT_GT(detected_peaks.back(), 350u);
}
