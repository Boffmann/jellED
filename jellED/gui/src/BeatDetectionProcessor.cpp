#include "BeatDetectionProcessor.h"

#include <cmath>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <set>
#include <thread>
#include "AudioDisplay.h"
#include "include/tempoTracker.h"
#include "include/sampleRecorder.h"

// --- Beat Timing Debug (for cross-platform comparison) ---
// Enable this to output beat detection timestamps in a format that can be compared with Raspberry Pi
static constexpr bool ENABLE_BEAT_TIMING_DEBUG = true;

static std::chrono::steady_clock::time_point beatTimingStartTime;
static bool beatTimingStartTimeInitialized = false;
static uint32_t beatTimingBeatCount = 0;

static void logBeatTiming(double internalTime) {
    if (!beatTimingStartTimeInitialized) {
        beatTimingStartTime = std::chrono::steady_clock::now();
        beatTimingStartTimeInitialized = true;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto wallClockMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    auto relativeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - beatTimingStartTime).count();
    
    beatTimingBeatCount++;
    
    std::cout << "[BEAT_MAC] beat#=" << beatTimingBeatCount 
              << " wall_ms=" << wallClockMs
              << " rel_ms=" << relativeMs
              << " internal_t=" << std::fixed << std::setprecision(4) << internalTime << "s"
              << std::endl;
}

// --- Audio Level Monitoring ---

static constexpr bool ENABLE_AUDIO_LEVEL_DEBUG = true;
static constexpr int AUDIO_LEVEL_REPORT_INTERVAL_MS = 2000;  // Report every 2 seconds

// Lock-free audio stats accumulator for hot path
// Uses double buffering: main thread writes to 'active', print thread reads from 'snapshot'
struct AudioLevelStats {
    struct Stats {
        double minSample = 1.0;
        double maxSample = -1.0;
        double sumSquares = 0.0;
        double sumAbsolute = 0.0;
        double peakLevel = 0.0;
        uint32_t sampleCount = 0;
        
        double minAfterAGC = 1.0;
        double maxAfterAGC = -1.0;
        double sumSquaresAfterAGC = 0.0;
        double peakAfterAGC = 0.0;
        uint32_t afterAGCCount = 0;
        
        double lastAGCGain = 1.0;
        double minAGCGain = 10.0;
        double maxAGCGain = 0.1;
        double sumAGCGain = 0.0;
        uint32_t agcGainCount = 0;
        
        void reset() {
            minSample = 1.0;
            maxSample = -1.0;
            sumSquares = 0.0;
            sumAbsolute = 0.0;
            peakLevel = 0.0;
            sampleCount = 0;
            minAfterAGC = 1.0;
            maxAfterAGC = -1.0;
            sumSquaresAfterAGC = 0.0;
            peakAfterAGC = 0.0;
            afterAGCCount = 0;
            minAGCGain = 10.0;
            maxAGCGain = 0.1;
            sumAGCGain = 0.0;
            agcGainCount = 0;
        }
    };
    
    Stats active;    // Written by main thread (no lock needed - single writer)
    Stats snapshot;  // Copy for printing (protected by mutex)
    std::mutex snapshotMutex;
    
    // Lock-free: called from main audio thread
    void addRawSample(double sample) {
        if (sample < active.minSample) active.minSample = sample;
        if (sample > active.maxSample) active.maxSample = sample;
        double absSample = std::abs(sample);
        active.sumSquares += sample * sample;
        active.sumAbsolute += absSample;
        if (absSample > active.peakLevel) active.peakLevel = absSample;
        active.sampleCount++;
    }
    
    // Lock-free: called from main audio thread
    void addAfterAGCSample(double sample, double agcGain) {
        if (sample < active.minAfterAGC) active.minAfterAGC = sample;
        if (sample > active.maxAfterAGC) active.maxAfterAGC = sample;
        double absSample = std::abs(sample);
        active.sumSquaresAfterAGC += sample * sample;
        if (absSample > active.peakAfterAGC) active.peakAfterAGC = absSample;
        active.afterAGCCount++;
        
        active.lastAGCGain = agcGain;
        if (agcGain < active.minAGCGain) active.minAGCGain = agcGain;
        if (agcGain > active.maxAGCGain) active.maxAGCGain = agcGain;
        active.sumAGCGain += agcGain;
        active.agcGainCount++;
    }
    
    // Called from print thread - takes snapshot and resets active
    void reset() {
        std::lock_guard<std::mutex> lock(snapshotMutex);
        snapshot = active;
        active.reset();
        active.lastAGCGain = snapshot.lastAGCGain;
    }
    
    // Called from print thread - prints the snapshot (call reset() first to take snapshot)
    void print() {
        std::lock_guard<std::mutex> lock(snapshotMutex);
        const Stats& s = snapshot;
        
        std::cout << "\n========== AUDIO LEVEL REPORT (Mac) ==========" << std::endl;
        std::cout << std::fixed << std::setprecision(6);
        
        if (s.sampleCount > 0) {
            double rms = std::sqrt(s.sumSquares / s.sampleCount);
            double avgAbs = s.sumAbsolute / s.sampleCount;
            double rmsDb = 20.0 * std::log10(std::max(rms, 1e-10));
            double peakDb = 20.0 * std::log10(std::max(s.peakLevel, 1e-10));
            
            std::cout << "[Raw Audio]" << std::endl;
            std::cout << "  Samples:    " << s.sampleCount << std::endl;
            std::cout << "  Range:      [" << s.minSample << " to " << s.maxSample << "]" << std::endl;
            std::cout << "  Peak:       " << s.peakLevel << " (" << std::setprecision(1) << peakDb << " dB)" << std::endl;
            std::cout << std::setprecision(6);
            std::cout << "  RMS:        " << rms << " (" << std::setprecision(1) << rmsDb << " dB)" << std::endl;
            std::cout << std::setprecision(6);
            std::cout << "  Avg |x|:    " << avgAbs << std::endl;
        }
        
        if (s.afterAGCCount > 0) {
            double rmsAgc = std::sqrt(s.sumSquaresAfterAGC / s.afterAGCCount);
            double rmsDbAgc = 20.0 * std::log10(std::max(rmsAgc, 1e-10));
            double peakDbAgc = 20.0 * std::log10(std::max(s.peakAfterAGC, 1e-10));
            
            std::cout << "[After AGC (normalized)]" << std::endl;
            std::cout << "  Samples:    " << s.afterAGCCount << std::endl;
            std::cout << "  Range:      [" << s.minAfterAGC << " to " << s.maxAfterAGC << "]" << std::endl;
            std::cout << "  Peak:       " << s.peakAfterAGC << " (" << std::setprecision(1) << peakDbAgc << " dB)" << std::endl;
            std::cout << std::setprecision(6);
            std::cout << "  RMS:        " << rmsAgc << " (" << std::setprecision(1) << rmsDbAgc << " dB)" << std::endl;
        }
        
        if (s.agcGainCount > 0) {
            double avgGain = s.sumAGCGain / s.agcGainCount;
            double avgGainDb = 20.0 * std::log10(std::max(avgGain, 1e-10));
            double minGainDb = 20.0 * std::log10(std::max(s.minAGCGain, 1e-10));
            double maxGainDb = 20.0 * std::log10(std::max(s.maxAGCGain, 1e-10));
            
            std::cout << "[AGC Gain]" << std::endl;
            std::cout << "  Current:    " << std::setprecision(2) << s.lastAGCGain << "x (" 
                      << std::setprecision(1) << (20.0 * std::log10(std::max(s.lastAGCGain, 1e-10))) << " dB)" << std::endl;
            std::cout << std::setprecision(2);
            std::cout << "  Range:      [" << s.minAGCGain << "x to " << s.maxAGCGain << "x] ("
                      << std::setprecision(1) << minGainDb << " to " << maxGainDb << " dB)" << std::endl;
            std::cout << std::setprecision(2);
            std::cout << "  Average:    " << avgGain << "x (" << std::setprecision(1) << avgGainDb << " dB)" << std::endl;
        }
        
        std::cout << "==============================================\n" << std::endl;
    }
};

static AudioLevelStats audioLevelStats;
static bool audioLevelThreadStarted = false;
static std::thread* audioLevelThread = nullptr;

BeatDetectionProcessor::BeatDetectionProcessor(
    AudioDisplay* display,
    jellED::SoundInput* soundInput,
    jellED::BeatDetector* beatDetector,
    double downsampleCutoffFrequency,
    double automaticGainControlTargetLevel,
    int signalDownsampleRatio,
    QObject* parent)
            : QThread(parent)
            , display_(display)
            , soundInput_(soundInput)
            , shouldStop_(false)
            , beatDetector_(beatDetector)
            , downsampler_(soundInput->getSampleRate(), signalDownsampleRatio, downsampleCutoffFrequency)
            , automaticGainControl_(soundInput->getSampleRate() / signalDownsampleRatio, automaticGainControlTargetLevel)
        {}

void BeatDetectionProcessor::run() {
    jellED::AudioBuffer buffer;

    jellED::TempoTracker tempoTracker;

    std::cout << "BeatDetectionProcessor running" << std::endl;
    
    // Start audio level reporting thread (only once)
    if constexpr (ENABLE_AUDIO_LEVEL_DEBUG) {
        if (!audioLevelThreadStarted) {
            audioLevelThreadStarted = true;
            audioLevelThread = new std::thread([]() {
                while(true) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(AUDIO_LEVEL_REPORT_INTERVAL_MS));
                    audioLevelStats.reset();  // Take snapshot first
                    audioLevelStats.print();  // Print from snapshot (doesn't block main thread)
                }
            });
        }
    }

    // jellED::SampleRecorder recorder(12000 * 5, "output.wav", 12000);
    
    while (!shouldStop_) {
        if (soundInput_->read(&buffer)) {
            // Collect raw audio level stats
            if constexpr (ENABLE_AUDIO_LEVEL_DEBUG) {
                for (size_t i = 0; i < buffer.num_samples; i++) {
                    audioLevelStats.addRawSample(buffer.buffer[i]);
                }
            }
            
            jellED::AudioBuffer downsampledBuffer;
            downsampler_.downsample(buffer, downsampledBuffer);
            for (size_t i = 0; i < downsampledBuffer.num_samples; i++) {

                // Apply Automatic Gain Control for consistent levels across environments
                //double sample = automaticGainControl_.apply(downsampledBuffer.buffer[i]);
                double sample = downsampledBuffer.buffer[i];

                // if (recorder.addSample(sample)) {
                //     // File was written (target reached)
                //     recorder.setEnabled(false);
                // }
                
                // Collect audio level stats after AGC
                if constexpr (ENABLE_AUDIO_LEVEL_DEBUG) {
                    audioLevelStats.addAfterAGCSample(sample, automaticGainControl_.getSectionGain());
                }
                
                display_->addOriginalSample(sample);

                const bool anyBeatDetected = this->beatDetector_->is_beat(sample);

                display_->addLowpassFilteredSampleLow(this->beatDetector_->getFilteredSampleLow());
                display_->addLowpassFilteredSampleMid(this->beatDetector_->getFilteredSampleMid());
                display_->addLowpassFilteredSampleHigh(this->beatDetector_->getFilteredSampleHigh());

                if (this->beatDetector_->getEnvelopeLow() != -1.0) {
                    display_->addEnvelopeFilteredSampleLow(this->beatDetector_->getEnvelopeLow());
                    if (this->beatDetector_->isPeakLow()) {
                        display_->addPeakLow();
                    }
                } else {
                    display_->addEnvelopeFilteredSampleLow(0.0);
                }

                if (this->beatDetector_->getEnvelopeMid() != -1.0) {
                    display_->addEnvelopeFilteredSampleMid(this->beatDetector_->getEnvelopeMid());
                    if (this->beatDetector_->isPeakMid()) {
                        display_->addPeakMid();
                    }
                } else {
                    display_->addEnvelopeFilteredSampleMid(0.0);
                }

                if (this->beatDetector_->getEnvelopeHigh() != -1.0) {
                    display_->addEnvelopeFilteredSampleHigh(this->beatDetector_->getEnvelopeHigh());
                    if (this->beatDetector_->isPeakHigh()) {
                        display_->addPeakHigh();
                    }
                } else {
                    display_->addEnvelopeFilteredSampleHigh(0.0);
                }

                if (anyBeatDetected) {
                    if constexpr (ENABLE_BEAT_TIMING_DEBUG) {
                        logBeatTiming(this->beatDetector_->getCurrentTime());
                    }
                    tempoTracker.addBeat(this->beatDetector_->getCurrentTime());
                    display_->addCombinedPeak();
                    display_->addCurrentDetectedBpm(tempoTracker.currentBpm());
                }
            }
        }
    }
}
