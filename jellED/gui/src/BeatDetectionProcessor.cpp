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
            , bandpassFilterLow_(jellED::BANDPASS_FILTER_COEFFICIENTS_LOW)
            , bandpassFilterMid_(jellED::BANDPASS_FILTER_COEFFICIENTS_MID)
            , bandpassFilterHigh_(jellED::BANDPASS_FILTER_COEFFICIENTS_HIGH)
            , envelopeDetectorLow_(builder.usbMicro_->getSampleRate() / builder.signalDownsampleRatio_, builder.envelopeDownsampleRatio_, builder.noveltyGain_)
            , envelopeDetectorMid_(builder.usbMicro_->getSampleRate() / builder.signalDownsampleRatio_, builder.envelopeDownsampleRatio_, builder.noveltyGain_)
            , envelopeDetectorHigh_(builder.usbMicro_->getSampleRate() / builder.signalDownsampleRatio_, builder.envelopeDownsampleRatio_, builder.noveltyGain_)
            , peakDetectorLow_(builder.peakDetectionAbsoluteMinThreshold_, builder.peakDetectionThresholdRel_, builder.peakDetectionMinPeakDistance_, builder.peakDetectionMaxBpm_, builder.usbMicro_->getSampleRate() / builder.signalDownsampleRatio_)
            , peakDetectorMid_(builder.peakDetectionAbsoluteMinThreshold_, builder.peakDetectionThresholdRel_, builder.peakDetectionMinPeakDistance_, builder.peakDetectionMaxBpm_, builder.usbMicro_->getSampleRate() / builder.signalDownsampleRatio_)
            , peakDetectorHigh_(builder.peakDetectionAbsoluteMinThreshold_, builder.peakDetectionThresholdRel_, builder.peakDetectionMinPeakDistance_, builder.peakDetectionMaxBpm_, builder.usbMicro_->getSampleRate() / builder.signalDownsampleRatio_)
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
                double filteredSampleLow = bandpassFilterLow_.apply(amplifiedSample);
                double filteredSampleMid = bandpassFilterMid_.apply(amplifiedSample);
                double filteredSampleHigh = bandpassFilterHigh_.apply(amplifiedSample);
                display_->addLowpassFilteredSampleLow(filteredSampleLow);
                display_->addLowpassFilteredSampleMid(filteredSampleMid);
                display_->addLowpassFilteredSampleHigh(filteredSampleHigh);
                double envelopeSampleLow = envelopeDetectorLow_.apply(filteredSampleLow);
                double envelopeSampleMid = envelopeDetectorMid_.apply(filteredSampleMid);
                double envelopeSampleHigh = envelopeDetectorHigh_.apply(filteredSampleHigh);
                if (envelopeSampleLow != -1.0) {
                    display_->addEnvelopeFilteredSampleLow(envelopeSampleLow);
                    double current_time = static_cast<double>(totalSamplesReceived_) / (usbMicro_->getSampleRate() / signalDownsampleRatio_);
                    if (peakDetectorLow_.is_peak(envelopeSampleLow, current_time)) {
                        display_->addPeakLow();
                    }
                } else {
                    display_->addEnvelopeFilteredSampleLow(0.0);
                }
                if (envelopeSampleMid != -1.0) {
                    display_->addEnvelopeFilteredSampleMid(envelopeSampleMid);
                    double current_time = static_cast<double>(totalSamplesReceived_) / (usbMicro_->getSampleRate() / signalDownsampleRatio_);
                    if (peakDetectorMid_.is_peak(envelopeSampleMid, current_time)) {
                        display_->addPeakMid();
                    }
                } else {
                    display_->addEnvelopeFilteredSampleMid(0.0);
                }
                if (envelopeSampleHigh != -1.0) {
                    display_->addEnvelopeFilteredSampleHigh(envelopeSampleHigh);
                    double current_time = static_cast<double>(totalSamplesReceived_) / (usbMicro_->getSampleRate() / signalDownsampleRatio_);
                    if (peakDetectorHigh_.is_peak(envelopeSampleHigh, current_time)) {
                        display_->addPeakHigh();
                    }
                } else {
                    display_->addEnvelopeFilteredSampleHigh(0.0);
                }
            }
        }
    }
}
