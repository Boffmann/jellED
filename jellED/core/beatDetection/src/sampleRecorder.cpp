#include "include/sampleRecorder.h"

#include <fstream>
#include <cmath>
#include <algorithm>

namespace jellED {

SampleRecorder::SampleRecorder(uint32_t targetSampleCount, const std::string& filePath, uint32_t sampleRate)
    : targetSampleCount_{targetSampleCount}
    , filePath_{filePath}
    , sampleRate_{sampleRate}
    , enabled_{true}
{
    samples_.reserve(targetSampleCount);
}

SampleRecorder::~SampleRecorder() {
    // Optionally write remaining samples on destruction
    // (currently we don't to avoid unexpected side effects)
}

bool SampleRecorder::addSample(double sample) {
    if (!enabled_) {
        return false;
    }

    samples_.push_back(sample);

    if (samples_.size() >= targetSampleCount_) {
        bool success = writeToFile();
        clear();
        return success;
    }

    return false;
}

uint32_t SampleRecorder::getCurrentSampleCount() const {
    return static_cast<uint32_t>(samples_.size());
}

uint32_t SampleRecorder::getTargetSampleCount() const {
    return targetSampleCount_;
}

bool SampleRecorder::isReady() const {
    return samples_.size() >= targetSampleCount_;
}

void SampleRecorder::setTargetSampleCount(uint32_t targetSampleCount) {
    targetSampleCount_ = targetSampleCount;
    samples_.clear();
    samples_.reserve(targetSampleCount);
}

void SampleRecorder::setFilePath(const std::string& filePath) {
    filePath_ = filePath;
}

const std::string& SampleRecorder::getFilePath() const {
    return filePath_;
}

void SampleRecorder::setSampleRate(uint32_t sampleRate) {
    sampleRate_ = sampleRate;
}

uint32_t SampleRecorder::getSampleRate() const {
    return sampleRate_;
}

void SampleRecorder::writeWavHeader(std::ofstream& file, uint32_t dataSize) {
    uint32_t byteRate = sampleRate_ * NUM_CHANNELS * BITS_PER_SAMPLE / 8;
    uint16_t blockAlign = NUM_CHANNELS * BITS_PER_SAMPLE / 8;
    uint32_t chunkSize = 36 + dataSize;  // File size - 8 bytes for RIFF header

    // RIFF chunk descriptor
    file.write("RIFF", 4);
    file.write(reinterpret_cast<const char*>(&chunkSize), 4);
    file.write("WAVE", 4);

    // fmt sub-chunk
    file.write("fmt ", 4);
    uint32_t subchunk1Size = 16;  // PCM format
    file.write(reinterpret_cast<const char*>(&subchunk1Size), 4);
    file.write(reinterpret_cast<const char*>(&AUDIO_FORMAT_PCM), 2);
    file.write(reinterpret_cast<const char*>(&NUM_CHANNELS), 2);
    file.write(reinterpret_cast<const char*>(&sampleRate_), 4);
    file.write(reinterpret_cast<const char*>(&byteRate), 4);
    file.write(reinterpret_cast<const char*>(&blockAlign), 2);
    file.write(reinterpret_cast<const char*>(&BITS_PER_SAMPLE), 2);

    // data sub-chunk
    file.write("data", 4);
    file.write(reinterpret_cast<const char*>(&dataSize), 4);
}

int16_t SampleRecorder::convertTo16BitPCM(double sample) const {
    // Clamp to [-1.0, 1.0]
    double clamped = std::max(-1.0, std::min(1.0, sample));
    
    // Scale to 16-bit range [-32768, 32767]
    // Using 32767 for positive values to avoid overflow
    if (clamped >= 0.0) {
        return static_cast<int16_t>(clamped * 32767.0);
    } else {
        return static_cast<int16_t>(clamped * 32768.0);
    }
}

bool SampleRecorder::writeToFile() {
    if (samples_.empty()) {
        return false;
    }

    std::ofstream file(filePath_, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // Calculate data size (16-bit = 2 bytes per sample)
    uint32_t dataSize = static_cast<uint32_t>(samples_.size()) * sizeof(int16_t);

    // Write WAV header
    writeWavHeader(file, dataSize);

    // Convert and write samples
    for (const double& sample : samples_) {
        int16_t pcmSample = convertTo16BitPCM(sample);
        file.write(reinterpret_cast<const char*>(&pcmSample), sizeof(int16_t));
    }

    file.close();
    return file.good();
}

void SampleRecorder::clear() {
    samples_.clear();
}

bool SampleRecorder::isEnabled() const {
    return enabled_;
}

void SampleRecorder::setEnabled(bool enabled) {
    enabled_ = enabled;
}

} // namespace jellED
