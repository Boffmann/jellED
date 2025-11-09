#include "BeatDetectionProcessor.h"

#include <iostream>
#include "AudioDisplay.h"

BeatDetectionProcessor::BeatDetectionProcessor(AudioDisplay* display,
    jellED::UsbMicro* usbMicro,
    int signalDownsampleRatio,
    int envelopeDownsampleRatio,
    QObject* parent)
            : QThread(parent)
            , display_(display)
            , usbMicro_(usbMicro)
            , signalDownsampleRatio_(signalDownsampleRatio)
            , shouldStop_(false)
            , adaptiveNormalizer_(usbMicro_->getSampleRate() / signalDownsampleRatio, 0.1, 0.01)
            , downsampler_(signalDownsampleRatio_, usbMicro_->getSampleRate(), 0.5)
            , bandpassFilter_()
            , envelopeDetector_(usbMicro_->getSampleRate() / signalDownsampleRatio, envelopeDownsampleRatio)
            , peakDetector_(0.01, 0.1, 0.1, 0.4, 180.0, usbMicro_->getSampleRate() / signalDownsampleRatio)
        {}

void BeatDetectionProcessor::run() {
    jellED::AudioBuffer buffer;
    const double GAIN = 2.0;

    std::cout << "BeatDetectionProcessor running" << std::endl;
    
    while (!shouldStop_) {
        if (usbMicro_->read(&buffer)) {
            jellED::AudioBuffer downsampledBuffer;
            downsampler_.downsample(buffer, downsampledBuffer);
            for (int i = 0; i < downsampledBuffer.num_samples; i++) {
                totalSamplesReceived_++;
                double adaptiveNormalizedSample = adaptiveNormalizer_.apply(downsampledBuffer.buffer[i]);
        //         // double adaptiveNormalizedSample = downsampledBuffer.buffer[i];
                double amplifiedSample = adaptiveNormalizedSample;// * GAIN / 2.0;
                display_->addOriginalSample(amplifiedSample);
                double filteredSample = bandpassFilter_.apply(amplifiedSample);// * GAIN;
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

void BeatDetectionProcessor::updateParameters(int signalDownsampleRatio, int envelopeDownsampleRatio) {
    signalDownsampleRatio_ = signalDownsampleRatio;
    adaptiveNormalizer_ = jellED::AdaptiveNormalizer(usbMicro_->getSampleRate() / signalDownsampleRatio, 0.1, 0.01);
    downsampler_ = jellED::Downsampler(signalDownsampleRatio_, usbMicro_->getSampleRate(), 0.5);
    envelopeDetector_ = jellED::EnvelopeDetector(usbMicro_->getSampleRate() / signalDownsampleRatio, envelopeDownsampleRatio);
    peakDetector_ = jellED::PeakDetector(0.01, 0.1, 0.1, 0.4, 180.0, usbMicro_->getSampleRate() / signalDownsampleRatio);
}