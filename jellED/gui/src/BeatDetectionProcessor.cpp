#include "BeatDetectionProcessor.h"

#include <iostream>
#include "AudioDisplay.h"

BeatDetectionProcessor::BeatDetectionProcessor(AudioDisplay* display,
    jellED::UsbMicro* usbMicro,
    int signalDownsampleRatio,
    int envelopeDownsampleRatio,
    double downsampleCutoffFrequency,
    double automaticGainControlTargetLevel,
    double peakDetectionAbsoluteMinThreshold,
    double peakDetectionThresholdRel,
    double peakDetectionMinPeakDistance,
    double peakDetectionMaxBpm,
    QObject* parent)
            : QThread(parent)
            , display_(display)
            , usbMicro_(usbMicro)
            , shouldStop_(false)
            , signalDownsampleRatio_(signalDownsampleRatio)
            , downsampler_(signalDownsampleRatio_, usbMicro_->getSampleRate(), downsampleCutoffFrequency)
            , automaticGainControl_(usbMicro_->getSampleRate() / signalDownsampleRatio, automaticGainControlTargetLevel)
            , bandpassFilter_()
            , envelopeDetector_(usbMicro_->getSampleRate() / signalDownsampleRatio, envelopeDownsampleRatio)
            , peakDetector_(peakDetectionAbsoluteMinThreshold, peakDetectionThresholdRel, peakDetectionMinPeakDistance, peakDetectionMaxBpm, usbMicro_->getSampleRate() / signalDownsampleRatio)
        {
            std::cout << "BeatDetectionProcessor constructed with signalDownsampleRatio: " << signalDownsampleRatio
            << ", envelopeDownsampleRatio: " << envelopeDownsampleRatio
            << ", downsampleCutoffFrequency: " << downsampleCutoffFrequency
            << ", automaticGainControlTargetLevel: " << automaticGainControlTargetLevel
            << ", peakDetectionAbsoluteMinThreshold: " << peakDetectionAbsoluteMinThreshold
            << ", peakDetectionThresholdRel: " << peakDetectionThresholdRel
            << ", peakDetectionMinPeakDistance: " << peakDetectionMinPeakDistance
            << ", peakDetectionMaxBpm: " << peakDetectionMaxBpm
            << std::endl;
        }

void BeatDetectionProcessor::run() {
    jellED::AudioBuffer buffer;

    std::cout << "BeatDetectionProcessor running" << std::endl;
    
    while (!shouldStop_) {
        if (usbMicro_->read(&buffer)) {
            jellED::AudioBuffer downsampledBuffer;
            downsampler_.downsample(buffer, downsampledBuffer);
            for (size_t i = 0; i < downsampledBuffer.num_samples; i++) {
                totalSamplesReceived_++;
                double sample = downsampledBuffer.buffer[i];
                double amplifiedSample = automaticGainControl_.apply(sample);
                display_->addOriginalSample(amplifiedSample);
                double filteredSample = bandpassFilter_.apply(amplifiedSample);
                display_->addLowpassFilteredSample(filteredSample);
                double envelopeSample = envelopeDetector_.apply(filteredSample);
                if (envelopeSample != -1.0) {
                    display_->addEnvelopeFilteredSample(envelopeSample);
                    double current_time = static_cast<double>(totalSamplesReceived_) / (usbMicro_->getSampleRate() / signalDownsampleRatio_);
                    if (peakDetector_.is_peak(envelopeSample, current_time)) {
                        std::cout << "Peak detected at time: " << current_time << std::endl;
                        display_->addPeak();
                    }
                } else {
                    display_->addEnvelopeFilteredSample(0.0);
                }
            }
        }
    }
}
