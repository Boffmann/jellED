#include "include/peakdetection.h"

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>

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

  jellED::PeakDetector peakDetector(0.01, 0.1, 0.1, 0.4, 180.0, scaled_sample_rate);

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
