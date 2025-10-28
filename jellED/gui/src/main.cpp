#include <QApplication>
#include <QThread>
#include <chrono>
#include "AudioDisplay.h"
#include "include/bandpassFilter.h"
#include "sound/raspi/usbMicro.h"


//std::string microphone_device_id = "BuiltInMicrophoneDevice";
std::string microphone_device_id = "AppleUSBAudioEngine:C-Media Electronics Inc.      :USB PnP Sound Device:2140000:1";

class BeatDetectorThread : public QThread {
public:
    BeatDetectorThread(AudioDisplay* display, jellED::UsbMicro* usbMicro, QObject* parent = nullptr)
        : QThread(parent)
        , display_(display)
        , usbMicro_(usbMicro)
        , shouldStop_(false)
        , bandpassFilter_()
    {}

    void stop() {
        shouldStop_ = true;
    }

protected:
    void run() override {
        jellED::AudioBuffer buffer;
        while (!shouldStop_) {
            if (usbMicro_->read(&buffer)) {
                for (int i = 0; i < buffer.num_samples; i++) {
                    display_->addOriginalSample(buffer.buffer[i]);
                    double filteredSample = bandpassFilter_.apply(buffer.buffer[i]);
                    display_->addLowpassFilteredSample(filteredSample);
                }
            }
        }
        // while (!shouldStop_) {
        //     int sign = rand() % 2 == 0 ? 1 : -1;
        //     double r = sign * static_cast <double> (rand()) / static_cast <double> (RAND_MAX);
        //     display_->addOriginalSample(r);
        //     double r2 = sign * static_cast <double> (rand()) / static_cast <double> (RAND_MAX);
        //     display_->addLowpassFilteredSample(r2);
        //     QThread::usleep(1000000 / sampleRate_);
        //     //QThread::msleep(11);
        // }
    }

private:
    AudioDisplay* display_;
    jellED::UsbMicro* usbMicro_;
    bool shouldStop_;
    jellED::BandpassFilter bandpassFilter_;
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