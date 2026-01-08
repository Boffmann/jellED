#ifndef __SAMPLE_RECORDER_JELLED_H__
#define __SAMPLE_RECORDER_JELLED_H__

#include <stdint.h>
#include <string>
#include <vector>

namespace jellED {

/**
 * @brief Collects audio samples and stores them to a WAV file once the target count is reached.
 * 
 * The SampleRecorder accumulates samples until the configured target sample count is reached,
 * then automatically writes them to the specified file path as a playable WAV file.
 * Supports multiple recording sessions by resetting after each write.
 * 
 * Input samples are expected to be normalized floating-point values in the range [-1.0, 1.0].
 * Output is written as 16-bit PCM WAV format.
 */
class SampleRecorder {
public:
    /**
     * @brief Construct a new Sample Recorder
     * 
     * @param targetSampleCount Number of samples to collect before writing to file
     * @param filePath Path to the output WAV file where samples will be stored
     * @param sampleRate Sample rate in Hz (e.g., 44100, 48000)
     */
    SampleRecorder(uint32_t targetSampleCount, const std::string& filePath, uint32_t sampleRate);
    
    ~SampleRecorder();

    /**
     * @brief Add a sample to the recorder
     * 
     * @param sample The audio sample value to record (expected range: -1.0 to 1.0)
     * @return true if writing to file was triggered (target count reached)
     * @return false if more samples are still needed
     */
    bool addSample(double sample);

    /**
     * @brief Get the current number of collected samples
     * 
     * @return uint32_t Number of samples currently stored
     */
    uint32_t getCurrentSampleCount() const;

    /**
     * @brief Get the target sample count
     * 
     * @return uint32_t Target number of samples before file write
     */
    uint32_t getTargetSampleCount() const;

    /**
     * @brief Check if recorder is ready to write (has collected enough samples)
     * 
     * @return true if target sample count has been reached
     */
    bool isReady() const;

    /**
     * @brief Set a new target sample count
     * 
     * This will clear any currently collected samples.
     * 
     * @param targetSampleCount New target count
     */
    void setTargetSampleCount(uint32_t targetSampleCount);

    /**
     * @brief Set a new output file path
     * 
     * @param filePath New file path for output (should end in .wav)
     */
    void setFilePath(const std::string& filePath);

    /**
     * @brief Get the current file path
     * 
     * @return const std::string& Current output file path
     */
    const std::string& getFilePath() const;

    /**
     * @brief Set the sample rate for WAV output
     * 
     * @param sampleRate Sample rate in Hz
     */
    void setSampleRate(uint32_t sampleRate);

    /**
     * @brief Get the current sample rate
     * 
     * @return uint32_t Sample rate in Hz
     */
    uint32_t getSampleRate() const;

    /**
     * @brief Manually trigger writing samples to WAV file
     * 
     * Writes all currently collected samples regardless of whether
     * the target count has been reached.
     * 
     * @return true if write was successful
     * @return false if write failed
     */
    bool writeToFile();

    /**
     * @brief Clear all collected samples without writing
     */
    void clear();

    /**
     * @brief Check if recording is enabled
     * 
     * @return true if recorder will collect samples
     */
    bool isEnabled() const;

    /**
     * @brief Enable or disable recording
     * 
     * @param enabled Whether to enable sample collection
     */
    void setEnabled(bool enabled);

private:
    /**
     * @brief Write WAV file header
     * 
     * @param file Output file stream (must be opened in binary mode)
     * @param dataSize Size of the audio data in bytes
     */
    void writeWavHeader(std::ofstream& file, uint32_t dataSize);

    /**
     * @brief Clamp and convert a double sample to 16-bit PCM
     * 
     * @param sample Input sample (expected range: -1.0 to 1.0)
     * @return int16_t Clamped and scaled 16-bit value
     */
    int16_t convertTo16BitPCM(double sample) const;

    std::vector<double> samples_;
    uint32_t targetSampleCount_;
    std::string filePath_;
    uint32_t sampleRate_;
    bool enabled_;

    static constexpr uint16_t NUM_CHANNELS = 1;        // Mono
    static constexpr uint16_t BITS_PER_SAMPLE = 16;    // 16-bit PCM
    static constexpr uint16_t AUDIO_FORMAT_PCM = 1;    // PCM format
};

} // namespace jellED

#endif
