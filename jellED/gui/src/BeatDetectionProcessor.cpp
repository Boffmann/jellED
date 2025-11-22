#include "BeatDetectionProcessor.h"

#include <iostream>
#include <set>
#include "AudioDisplay.h"
#include "include/tempoTracker.h"

BeatDetectionProcessor::BeatDetectionProcessor(const Builder& builder)
            : QThread(builder.parent_)
            , display_(builder.display_)
            , usbMicro_(builder.usbMicro_)
            , shouldStop_(false)
            , signalDownsampleRatio_(builder.signalDownsampleRatio_)
            , totalSamplesReceived_(0)
            , downsampler_(builder.signalDownsampleRatio_, builder.usbMicro_->getSampleRate(), builder.downsampleCutoffFrequency_)
            , automaticGainControl_(builder.usbMicro_->getSampleRate() / builder.signalDownsampleRatio_, builder.automaticGainControlTargetLevel_)
            , bandConfigLow_{jellED::BANDPASS_FILTER_COEFFICIENTS_LOW, 280.0, 0.006, 0.22, 1.0}  // Increased weight for kick
            , bandConfigMid_{jellED::BANDPASS_FILTER_COEFFICIENTS_MID, 320.0, 0.010, 0.18, 0.35}
            , bandConfigHigh_{jellED::BANDPASS_FILTER_COEFFICIENTS_HIGH, 280.0, 0.006, 0.22, 0.4}  // Reduced weight for hi-hats
            , bandStateLow_(bandConfigLow_, usbMicro_->getSampleRate(), builder.envelopeDownsampleRatio_, builder.peakDetectionMaxBpm_)
            , bandStateMid_(bandConfigMid_, usbMicro_->getSampleRate(), builder.envelopeDownsampleRatio_, builder.peakDetectionMaxBpm_)
            , bandStateHigh_(bandConfigHigh_, usbMicro_->getSampleRate(), builder.envelopeDownsampleRatio_, builder.peakDetectionMaxBpm_)
            , multibandFusion_(0.15, builder.peakDetectionMaxBpm_)
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

    jellED::TempoTracker tempoTracker;

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
                double filteredSampleLow = bandStateLow_.applyBandpassFilter(amplifiedSample);
                double filteredSampleMid = bandStateMid_.applyBandpassFilter(amplifiedSample);
                double filteredSampleHigh = bandStateHigh_.applyBandpassFilter(amplifiedSample);
                display_->addLowpassFilteredSampleLow(filteredSampleLow);
                display_->addLowpassFilteredSampleMid(filteredSampleMid);
                display_->addLowpassFilteredSampleHigh(filteredSampleHigh);
                double envelopeSampleLow = bandStateLow_.applyEnvelopeFilter(filteredSampleLow);
                double envelopeSampleMid = bandStateMid_.applyEnvelopeFilter(filteredSampleMid);
                double envelopeSampleHigh = bandStateHigh_.applyEnvelopeFilter(filteredSampleHigh);

                double current_time = static_cast<double>(totalSamplesReceived_) / (usbMicro_->getSampleRate() / signalDownsampleRatio_);
                bool anyPeakDetected = false;

                if (envelopeSampleLow != -1.0) {
                    display_->addEnvelopeFilteredSampleLow(envelopeSampleLow);
                    double strength = bandStateLow_.applyPeakDetector(envelopeSampleLow, current_time);
                    if (strength > 0.0) {
                        display_->addPeakLow();
                        if (multibandFusion_.push({current_time, strength, 0})) {
                            anyPeakDetected = true;
                        }
                    }
                } else {
                    display_->addEnvelopeFilteredSampleLow(0.0);
                }
                if (envelopeSampleMid != -1.0) {
                    display_->addEnvelopeFilteredSampleMid(envelopeSampleMid);
                    double strength = bandStateMid_.applyPeakDetector(envelopeSampleMid, current_time);
                    if (strength > 0.0) {
                        display_->addPeakMid();
                        if (multibandFusion_.push({current_time, strength, 1})) {
                            anyPeakDetected = true;
                        }
                    }
                } else {
                    display_->addEnvelopeFilteredSampleMid(0.0);
                }
                if (envelopeSampleHigh != -1.0) {
                    display_->addEnvelopeFilteredSampleHigh(envelopeSampleHigh);
                    double strength = bandStateHigh_.applyPeakDetector(envelopeSampleHigh, current_time);
                    if (strength > 0.0) {
                        display_->addPeakHigh();
                        if (multibandFusion_.push({current_time, strength, 2})) {
                            anyPeakDetected = true;
                        }
                    }
                } else {
                    display_->addEnvelopeFilteredSampleHigh(0.0);
                }

                if (anyPeakDetected) {
                    tempoTracker.addBeat(current_time);
                    display_->addCombinedPeak();
                    display_->addCurrentDetectedBpm(tempoTracker.currentBpm());
                }
            }
        }
    }
}
