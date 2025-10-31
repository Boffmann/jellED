#include <QApplication>
#include <QThread>
#include <chrono>
#include "AudioDisplay.h"
#include "include/bandpassFilter.h"
#include "include/envelopeDetector.h"
#include "include/peakdetection.h"
#include "sound/raspi/usbMicro.h"

#include <iostream>

//std::string microphone_device_id = "BuiltInMicrophoneDevice";
std::string microphone_device_id = "AppleUSBAudioEngine:C-Media Electronics Inc.      :USB PnP Sound Device:2123000:1";

constexpr int ENVELOPE_DOWNSAMPLE_RATIO = 8;

class BeatDetectorThread : public QThread {
public:
    BeatDetectorThread(AudioDisplay* display, jellED::UsbMicro* usbMicro, QObject* parent = nullptr)
        : QThread(parent)
        , display_(display)
        , usbMicro_(usbMicro)
        , shouldStop_(false)
        , bandpassFilter_()
        , envelopeDetector_(usbMicro_->getSampleRate(), ENVELOPE_DOWNSAMPLE_RATIO)
        , peakDetector_(0.01, 0.1, 0.1, 0.4, 180.0, usbMicro_->getSampleRate())
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
                for (int i = 0; i < buffer.num_samples; i++) {
                    totalSamplesReceived_++;
                    double amplifiedSample = buffer.buffer[i] * GAIN / 2.0;
                    display_->addOriginalSample(amplifiedSample);
                    double filteredSample = bandpassFilter_.apply(amplifiedSample) * GAIN;
                    display_->addLowpassFilteredSample(filteredSample);
                    double envelopeSample = envelopeDetector_.apply(filteredSample);
                    if (envelopeSample != -1.0) {
                        display_->addEnvelopeFilteredSample(envelopeSample);
                        double current_time = static_cast<double>(totalSamplesReceived_) / usbMicro_->getSampleRate();
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
    jellED::BandpassFilter bandpassFilter_;
    jellED::EnvelopeDetector envelopeDetector_;
    jellED::PeakDetector peakDetector_;
};

int main(int argc, char* argv[]) {

    jellED::UsbMicro usbMicro(microphone_device_id);
    usbMicro.initialize();

     jellED::UsbMicro::print_available_input_devices();

    int sampleRate = usbMicro.getSampleRate();
    QApplication app(argc, argv);
    AudioDisplay display(sampleRate, 5, 15);
    display.show();

    BeatDetectorThread generator(&display, &usbMicro);
    generator.start();

    int result = app.exec();

    // Stop the generator thread
    generator.stop();
    generator.wait();

    return result;
}