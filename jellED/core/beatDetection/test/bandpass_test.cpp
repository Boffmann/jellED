#include "../include/bandpassFilter.h"

#include <algorithm>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <vector>

class BandpassFilterTest : public testing::Test {

protected:
  std::vector<double> samples;
  std::vector<double> expected_result;

  BandpassFilterTest() {
    // Setup work for each test
    read_resources();
  }

  ~BandpassFilterTest() {
    // Clanup work that doesn't throw exception
  }

  void read_resources() {
    // Get the path of the current source file
    std::string source_path = __FILE__;
    std::filesystem::path source_dir =
        std::filesystem::path(source_path).parent_path();

    std::filesystem::path original_samples_file_path =
        source_dir / "resources/beatDetection/unfiltered_test_values.txt";
    std::ifstream unfilteredFile(original_samples_file_path);

    if (!unfilteredFile.is_open()) {
        std::cerr << "Failed to open the unfiltered file: " << std::strerror(errno)
                  << std::endl;
        return;
    }

    std::filesystem::path filtered_samples_file_path =
        source_dir / "resources/beatDetection/filtered_test_values.txt";
    std::ifstream filteredFile(filtered_samples_file_path);

    if (!filteredFile.is_open()) {
      std::cerr << "Failed to open the filtered file: " << std::strerror(errno)
                << std::endl;
      return;
    }

    std::string line;
    while (getline(unfilteredFile, line)) {
      double sample = std::stod(line);
      samples.push_back(sample);
    }
    while (getline(filteredFile, line)) {
      double sample = std::stod(line);
      expected_result.push_back(sample);
    }
  }
};

TEST_F(BandpassFilterTest, TestApplyBandpass) {
  ASSERT_GT(samples.size(), 0);
  ASSERT_GT(expected_result.size(), 0);
  BandpassFilter filter;

  std::vector<double> actual_result;

  std::for_each(samples.begin(), samples.end(),
                [&actual_result, &filter](double &sample) {
                  actual_result.push_back(filter.apply(sample));
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
