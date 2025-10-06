#include "include/envelopeDetector.h"

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>

class EnvelopeFilterTest : public testing::Test {
  protected:
    std::vector<double> samples;
    std::vector<double> expected_result;

    EnvelopeFilterTest() {
        read_resources();
    }

    ~EnvelopeFilterTest() {

    }

    void read_resources() {
        // Get the path of the current source file
        std::string source_path = __FILE__;
        std::filesystem::path source_dir =
            std::filesystem::path(source_path).parent_path();

        std::filesystem::path filtered_samples_file_path =
            source_dir / "resources/filtered_test_values.txt";
        std::ifstream filteredFile(filtered_samples_file_path);

        if (!filteredFile.is_open()) {
            std::cerr << "Failed to open the filtered file: " << std::strerror(errno)
                        << std::endl;
            return;
        }

        std::filesystem::path envelope_samples_file_path =
            source_dir / "resources/enveloped_test_values.txt";
        std::ifstream envelopedFile(envelope_samples_file_path);

        if (!envelopedFile.is_open()) {
            std::cerr << "Failed to open the enveloped file: " << std::strerror(errno)
                        << std::endl;
            return;
        }

        std::string line;
        while (getline(filteredFile, line)) {
            double sample = std::stod(line);
            samples.push_back(sample);
        }
        while (getline(envelopedFile, line)) {
            double sample = std::stod(line);
            expected_result.push_back(sample);
        }
    }
};

TEST_F(EnvelopeFilterTest, TestApplyEnvelopeFilter) {
  ASSERT_GT(samples.size(), 0);
  ASSERT_GT(expected_result.size(), 0);
  jellED::EnvelopeDetector envelopeDetector(44100, 8);

  std::vector<double> actual_result;

  std::for_each(samples.begin(), samples.end(),
                [&actual_result, &envelopeDetector](double &sample) {
                  float envelope = envelopeDetector.apply(sample);
                  if (envelope != -1.0) {
                    actual_result.push_back(envelope);
                  }
                });

  testing::internal::CaptureStdout();
  std::cout << "Actual result size: " << actual_result.size() << std::endl;
  std::cout << "Expected result size: " << expected_result.size() << std::endl;

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
