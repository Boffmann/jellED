#include "pUtils/raspi/include/raspiutils.h"
#include "serial/raspi/RaspiUart.h"
#include "pushButton/pushbutton.h"
#include "sound/raspi/usbMicro.h"
#include "sound/audiobuffer.h"
#include "include/downsampler.h"
#include "include/sampleRecorder.h"
#include "beatdetection.h"
#include "uartProtocol.h"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <atomic>
#include <iomanip>

using namespace jellED;

static constexpr int SIGNAL_DOWNSAMPLE_RATIO = 4;
static constexpr int ENVELOPE_DOWNSAMPLE_RATIO = 1;
static constexpr double DOWNSAMPLE_CUTOFF_FREQUENCY = 0.5;

static constexpr double DEFAULT_NOVELTY_GAIN = 300.0;

static constexpr double AUTOMATIC_GAIN_CONTROL_TARGET_LEVEL = 0.4;

// With AGC enabled, thresholds should match Mac (AGC normalizes input levels)
static constexpr double PEAK_DETECTION_ABSOLUTE_MIN_THRESHOLD = 0.05;
static constexpr double PEAK_DETECTION_THRESHOLD_REL = 0.1;
static constexpr double PEAK_DETECTION_MIN_PEAK_DISTANCE = 0.4;
static constexpr double PEAK_DETECTION_MAX_BPM = 180.0;

constexpr uint32_t ESP_UART_BAUD_RATE = 115200;
constexpr bool MODE_SEND = true;
constexpr uint8_t PUSH_BUTTON_PIN = 4;

std::string microphone_device_id = "hw:CARD=Device,DEV=0";

RaspiPlatformUtils raspiUtils;
SerialConfig serialConfig;
RaspiUart uart("/dev/ttyS0", 0);

PushButton pushButton(raspiUtils, PUSH_BUTTON_PIN);

int beat_detected_count = 0;
void uart_send_int(const uint8_t dataToWrite) {
    auto start = std::chrono::steady_clock::now();
    
	if (uart.send(&dataToWrite, 1) == -1) {
		std::cout << "Failed to write to uart" << std::endl;
	} else {
		std::cout << "Beat sent via UART " << beat_detected_count << std::endl;
        beat_detected_count++;
	}
}

bool initializeMicrophone(UsbMicro& usbMicro) {
    const int timeout_seconds = 3;
    AudioBuffer buffer;
    bool got_data = false;
    usbMicro.initialize();

    auto start = std::chrono::steady_clock::now();

    while (!got_data) {
        got_data = usbMicro.read(&buffer);
        if (got_data) {
            std::cout << "Audio data received! Starting main processing loop." << std::endl;
            return true;
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();

        if (elapsed >= timeout_seconds) {
            std::cout << "Timeout reached (" << timeout_seconds << "s). Starting main loop anyway..." << std::endl;
            return false;
        }

        // Sleep briefly to avoid busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return false;
}

bool button_state = false;
void checkButtonPressed() {
    if (pushButton.isPressed()) {
        if (button_state == false) {
            // Button Pressed
            std::cout << "Button Pressed" << std::endl;
            uart_send_int(UART_BUTTON_PRESSED);
        }
        button_state = true;
    } else {
        if (button_state == true) {
            // Button Released
            std::cout << "Button Released" << std::endl;
        }
        button_state = false;
    }
}

void calculate_microphone_drain_time(UsbMicro* usbMicro) {
    AudioBuffer buffer;
    int totalSamples = 0;
    auto testStart = std::chrono::steady_clock::now();

    while (totalSamples < 48000) {
        bool gotAudio = usbMicro->read(&buffer);
        if (gotAudio && buffer.num_samples > 0) {
            totalSamples += buffer.num_samples;
        } else {
            // Buffer is empty - waiting for new audio
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    auto testEnd = std::chrono::steady_clock::now();
    auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(testEnd - testStart).count();
    std::cout << "Time to get 48000 samples: " << totalMs << " ms" << std::endl;
}

int main () {
    if (!uart.initialize(serialConfig, ESP_UART_BAUD_RATE)) {
    	std::cout << "Error setting up uart" << std::endl;
		return 1;
    }

    AudioBuffer buffer;

    // UsbMicro::print_available_input_devices(SoundIoBackendAlsa);
    UsbMicro usbMicro(microphone_device_id, SoundIoBackendAlsa);
    // UsbMicro usbMicro(microphone_device_id, SoundIoBackendPulseAudio);
    if (!initializeMicrophone(usbMicro)) {
        std::cout << "Error initializing Microphone" << std::endl;
        return 1;
    }

    // std::thread button_thread([]() {
    //     while(true) {
    //         checkButtonPressed();
    //         std::this_thread::sleep_for(std::chrono::milliseconds(20));
    //     }
    // });

    std::mutex beat_ready_mutex;
    std::condition_variable beat_ready_cv;
    bool beat_ready = false;
    std::chrono::steady_clock::time_point beat_detected_time;
    
    std::thread beat_uart_thread([&beat_ready_mutex, &beat_ready_cv, &beat_ready, &beat_detected_time]() {
        while(true) {
            std::chrono::steady_clock::time_point detected_time_copy;
            
            // Only hold lock while checking/resetting the flag - release before UART send
            {
                std::unique_lock<std::mutex> lock(beat_ready_mutex);
                beat_ready_cv.wait(lock, [&beat_ready] { return beat_ready; });
                detected_time_copy = beat_detected_time;
                beat_ready = false;  // Reset immediately so main thread can signal again
            }  // Lock released here - BEFORE UART send
            
            // Measure latency from beat detection to UART thread wake-up
            auto now = std::chrono::steady_clock::now();
            auto latency_us = std::chrono::duration_cast<std::chrono::microseconds>(now - detected_time_copy).count();
            if (latency_us > 1000) {  // Only log if > 1ms
                std::cout << "[Latency] Beat detect to UART thread: " << latency_us << " µs" << std::endl;
            }
            
            // UART send happens WITHOUT holding the lock
            uart_send_int(UART_BEAT_DETECTED);
        }
    });

    Downsampler downsampler(usbMicro.getSampleRate(), SIGNAL_DOWNSAMPLE_RATIO, DOWNSAMPLE_CUTOFF_FREQUENCY);
    AutomaticGainControl automaticGainControl(usbMicro.getSampleRate() / SIGNAL_DOWNSAMPLE_RATIO, AUTOMATIC_GAIN_CONTROL_TARGET_LEVEL);
    jellED::SampleRecorder recorder(12000 * 5, "output.wav", 12000);

    std::cout << "Microphone sample rate: " << usbMicro.getSampleRate() << " Hz" << std::endl;
    std::cout << "Beat detection sample rate: " << (usbMicro.getSampleRate() / SIGNAL_DOWNSAMPLE_RATIO) << " Hz" << std::endl;

    BeatDetector* beatDetector = BeatDetector::Builder(usbMicro.getSampleRate() / SIGNAL_DOWNSAMPLE_RATIO)
        .setEnvelopeDownsampleRatio(ENVELOPE_DOWNSAMPLE_RATIO)
        .setNoveltyGain(DEFAULT_NOVELTY_GAIN)
        .setPeakDetectionAbsoluteMinThreshold(PEAK_DETECTION_ABSOLUTE_MIN_THRESHOLD)
        .setPeakDetectionThresholdRel(PEAK_DETECTION_THRESHOLD_REL)
        .setPeakDetectionMinPeakDistance(PEAK_DETECTION_MIN_PEAK_DISTANCE)
        .setPeakDetectionMaxBpm(PEAK_DETECTION_MAX_BPM)
        .build();


    usbMicro.drain_audio_buffer();
    // calculate_microphone_drain_time(&usbMicro);
    // calculate_microphone_drain_time(&usbMicro);
    // calculate_microphone_drain_time(&usbMicro);
    // calculate_microphone_drain_time(&usbMicro);
    // calculate_microphone_drain_time(&usbMicro);
    // calculate_microphone_drain_time(&usbMicro);
    // calculate_microphone_drain_time(&usbMicro);
    // calculate_microphone_drain_time(&usbMicro);

    std::cout << "Starting main audio processing loop..." << std::endl;

    int samplesRecordedTime = 0;
    long samplesRecordedTimeTotal = 0;

    int totalSamplesRead = 0;
    int readAudioTimeTotal = 0;
    int flushCounter = 0;
    while(true) {
        auto readAudioStart = std::chrono::steady_clock::now();
        bool gotAudio = usbMicro.read(&buffer);
        // Flush events periodically to process audio system events
        if (++flushCounter >= 750) {  // Every ~750 iterations ≈ 100ms at 64 samples/read
            usbMicro.flushEvents();
            flushCounter = 0;
        }
        if (gotAudio) {
            auto readAudioEnd = std::chrono::steady_clock::now();
            uint32_t readAudioTimeUs = static_cast<uint32_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(readAudioEnd - readAudioStart).count());
            readAudioTimeTotal += readAudioTimeUs;
            totalSamplesRead+=buffer.num_samples;
            if (totalSamplesRead >= 48000) {
                std::cout << "Read audio took: " << readAudioTimeTotal << " Us" << std::endl;
                readAudioTimeTotal = 0;
                totalSamplesRead = 0;
            }
        }
        auto start = std::chrono::steady_clock::now();

        if (gotAudio) {
            jellED::AudioBuffer downsampledBuffer;
            downsampler.downsample(buffer, downsampledBuffer);

            for (int i = 0; i < downsampledBuffer.num_samples; i++) {
                samplesRecordedTime++;
                // Apply Automatic Gain Control for consistent levels across environments
                // double sample = automaticGainControl.apply(downsampledBuffer.buffer[i]);
                double sample = downsampledBuffer.buffer[i];
                if (recorder.addSample(sample)) {
                    // File was written (target reached)
                    recorder.setEnabled(false);
                }

                bool isBeat = beatDetector->is_beat(sample);
                // if (recorder.addSample(beatDetector->getFilteredSampleLow())) {
                //     // File was written (target reached)
                //     recorder.setEnabled(false);
                // }
                // Only process for beats if audio passes noise gate
                if (isBeat) {
                    std::cout << "Detected a beat!!" << std::endl;
                    {
                        std::lock_guard<std::mutex> lock(beat_ready_mutex);
                        beat_detected_time = std::chrono::steady_clock::now();
                        beat_ready = true;
                    }
                    beat_ready_cv.notify_one();
                }
            }
        }
        auto end = std::chrono::steady_clock::now();
        uint32_t beatDetectionTimeUs = static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        samplesRecordedTimeTotal += beatDetectionTimeUs;
        if (samplesRecordedTime >= 6000) {

            std::cout << "Beat detection time: " << samplesRecordedTimeTotal / samplesRecordedTime << " µs" << std::endl;
            samplesRecordedTime = 0;
            samplesRecordedTimeTotal = 0;
        }
    }
}
