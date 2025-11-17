#include "BeatDetectionProcessor.h"

#include <iostream>
#include "AudioDisplay.h"

BeatDetectionProcessor::BeatDetectionProcessor(const Builder& builder)
            : QThread(builder.parent_)
            , display_(builder.display_)
            , usbMicro_(builder.usbMicro_)
            , shouldStop_(false)
            , signalDownsampleRatio_(builder.signalDownsampleRatio_)
            , totalSamplesReceived_(0)
            , downsampler_(builder.signalDownsampleRatio_, builder.usbMicro_->getSampleRate(), builder.downsampleCutoffFrequency_)
            , automaticGainControl_(builder.usbMicro_->getSampleRate() / builder.signalDownsampleRatio_, builder.automaticGainControlTargetLevel_)
            , bandpassFilter_()
            , envelopeDetector_(builder.usbMicro_->getSampleRate() / builder.signalDownsampleRatio_, builder.envelopeDownsampleRatio_, builder.noveltyGain_)
            , peakDetector_(builder.peakDetectionAbsoluteMinThreshold_, builder.peakDetectionThresholdRel_, builder.peakDetectionMinPeakDistance_, builder.peakDetectionMaxBpm_, builder.usbMicro_->getSampleRate() / builder.signalDownsampleRatio_)
        {
            std::cout << "BeatDetectionProcessor constructed with signalDownsampleRatio: " << builder.signalDownsampleRatio_
            << ", envelopeDownsampleRatio: " << builder.envelopeDownsampleRatio_
            << ", noveltyGain: " << builder.noveltyGain_
            << ", downsampleCutoffFrequency: " << builder.downsampleCutoffFrequency_
            << ", automaticGainControlTargetLevel: " << builder.automaticGainControlTargetLevel_
            << ", peakDetectionAbsoluteMinThreshold: " << builder.peakDetectionAbsoluteMinThreshold_
            << ", peakDetectionThresholdRel: " << builder.peakDetectionThresholdRel_
            << ", peakDetectionMinPeakDistance: " << builder.peakDetectionMinPeakDistance_
            << ", peakDetectionMaxBpm: " << builder.peakDetectionMaxBpm_
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
                // double amplifiedSample = automaticGainControl_.apply(sample);
                double amplifiedSample = sample;
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
