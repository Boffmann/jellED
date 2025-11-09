#include <QApplication>
#include <QThread>
#include <chrono>
#include "AudioDisplay.h"

#include <iostream>

std::string microphone_device_id = "BuiltInMicrophoneDevice";
//std::string microphone_device_id = "AppleUSBAudioEngine:C-Media Electronics Inc.      :USB PnP Sound Device:2123000:1";

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    AudioDisplay display(microphone_device_id, 5, 30);
    display.startBeatDetectionProcessor();
    display.show();

    int result = app.exec();

    return result;
}