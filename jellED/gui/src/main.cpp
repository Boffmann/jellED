#include "AudioDisplay.h"
#include <QApplication>
#include <QThread>
#include <chrono>

#include <iostream>

// std::string microphone_device_id = "AppleUSBAudioEngine:AT:AT2020USB-X:202011110001:1";
// std::string microphone_device_id = "AppleUSBAudioEngine:C-Media Electronics
std::string microphone_device_id = "BuiltInMicrophoneDevice";
// Inc.:USB PnP Sound Device:2122000:1";

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  AudioDisplay display(microphone_device_id, 5, 30);
  display.startBeatDetectionProcessor();
  display.show();

  int result = app.exec();

  return result;
}
