#include <QApplication>
#include <QThread>
#include <chrono>
#include "AudioDisplay.h"
#include "include/bandpassFilter.h"
#include "include/bandpassFilter.h"
#include "include/envelopeDetector.h"
#include "include/peakdetection.h"
#include "include/adaptiveNormalizer.h"
#include "include/downsampler.h"
#include "sound/raspi/usbMicro.h"

#include <iostream>

std::string microphone_device_id = "BuiltInMicrophoneDevice";
//std::string microphone_device_id = "AppleUSBAudioEngine:C-Media Electronics Inc.      :USB PnP Sound Device:2123000:1";

constexpr int SIGNAL_DOWNSAMPLE_RATIO = 4;
constexpr int ENVELOPE_DOWNSAMPLE_RATIO = 8;

class BeatDetectorThread : public QThread {
public:
    BeatDetectorThread(AudioDisplay* display, jellED::UsbMicro* usbMicro, QObject* parent = nullptr)
        : QThread(parent)
        , display_(display)
        , usbMicro_(usbMicro)
        , shouldStop_(false)
        , adaptiveNormalizer_(usbMicro_->getSampleRate() / SIGNAL_DOWNSAMPLE_RATIO, 0.1, 0.01)
        , downsampler_(SIGNAL_DOWNSAMPLE_RATIO, usbMicro_->getSampleRate(), 0.5)
        , bandpassFilter_()
        , envelopeDetector_(usbMicro_->getSampleRate() / SIGNAL_DOWNSAMPLE_RATIO, ENVELOPE_DOWNSAMPLE_RATIO)
        , peakDetector_(0.01, 0.1, 0.1, 0.4, 180.0, usbMicro_->getSampleRate() / SIGNAL_DOWNSAMPLE_RATIO)
    {}

    void stop() {
        shouldStop_ = true;
    }

protected:
    void run() override {
        jellED::AudioBuffer buffer;
        const double GAIN = 2.0;
        while (!shouldStop_) {
            if (usbMicro_->read(&buffer)) {
                jellED::AudioBuffer downsampledBuffer;
                downsampler_.downsample(buffer, downsampledBuffer);
                // std::cout << "Downsampled buffer size: " << downsampledBuffer.num_samples << std::endl;
                for (int i = 0; i < downsampledBuffer.num_samples; i++) {
                    totalSamplesReceived_++;
                    double adaptiveNormalizedSample = adaptiveNormalizer_.apply(downsampledBuffer.buffer[i]);
                    // double adaptiveNormalizedSample = downsampledBuffer.buffer[i];
                    double amplifiedSample = adaptiveNormalizedSample;// * GAIN / 2.0;
                    display_->addOriginalSample(amplifiedSample);
                    double filteredSample = bandpassFilter_.apply(amplifiedSample);// * GAIN;
                    display_->addLowpassFilteredSample(filteredSample);
                    double envelopeSample = envelopeDetector_.apply(filteredSample);
                    if (envelopeSample != -1.0) {
                        display_->addEnvelopeFilteredSample(envelopeSample);
                        double current_time = static_cast<double>(totalSamplesReceived_) / (usbMicro_->getSampleRate() / SIGNAL_DOWNSAMPLE_RATIO);
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

private:
    AudioDisplay* display_;
    jellED::UsbMicro* usbMicro_;
    bool shouldStop_;
    uint32_t totalSamplesReceived_;
    jellED::AdaptiveNormalizer adaptiveNormalizer_;
    jellED::BandpassFilter bandpassFilter_;
    jellED::Downsampler downsampler_;
    jellED::EnvelopeDetector envelopeDetector_;
    jellED::PeakDetector peakDetector_;
};

int main(int argc, char* argv[]) {

    jellED::UsbMicro usbMicro(microphone_device_id, SoundIoBackendCoreAudio);
    usbMicro.initialize();

     jellED::UsbMicro::print_available_input_devices(SoundIoBackendCoreAudio);

    int sampleRate = usbMicro.getSampleRate() / SIGNAL_DOWNSAMPLE_RATIO;
    std::cout << "Sample rate: " << sampleRate << std::endl;
    QApplication app(argc, argv);
    AudioDisplay display(sampleRate, 5, 30);
    display.show();

    BeatDetectorThread generator(&display, &usbMicro);
    generator.start();

    int result = app.exec();

    // Stop the generator thread
    generator.stop();
    generator.wait();

    return result;
}