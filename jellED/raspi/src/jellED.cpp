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
#include <algorithm>
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

constexpr uint32_t ESP_UART_BAUD_RATE = 115200;
constexpr bool MODE_SEND = true;
constexpr uint8_t PUSH_BUTTON_PIN = 4;

// Volume quantization scale: maps envelope amplitude [0, 0.5] → [0, 255].
// Tune if LEDs are too dim (lower) or always saturate (raise).
static constexpr double VOLUME_SCALE = 512.0;

// Send a volume packet every N downsampled samples (12 kHz / 25 Hz = 480).
static constexpr int VOLUME_UPDATE_INTERVAL = 480;

std::string microphone_device_id = "hw:CARD=Device,DEV=0";

RaspiPlatformUtils raspiUtils;
SerialConfig serialConfig;
RaspiUart uart("/dev/ttyS0", 0);

PushButton pushButton(raspiUtils, PUSH_BUTTON_PIN);

// ── Packet helpers ────────────────────────────────────────────────────────────

static int packets_sent = 0;

void uart_send_features(const UartFeatures& features) {
    uint8_t packet[UART_PACKET_SIZE];
    uart_build_packet(features, packet);
    if (uart.send(packet, UART_PACKET_SIZE) == -1) {
        std::cout << "Failed to write to uart" << std::endl;
    } else {
        packets_sent++;
    }
}

// Quantise double volume [0, ∞) to [0, 255] using VOLUME_SCALE.
static inline uint8_t quantizeVolume(double v) {
    return static_cast<uint8_t>(std::min(255.0, std::max(0.0, v * VOLUME_SCALE)));
}

// Quantise spectral tilt [-1, 1] to [0, 255]:  0=treble-heavy, 255=bass-heavy.
static inline uint8_t quantizeTilt(double tilt) {
    return static_cast<uint8_t>(std::min(255.0, std::max(0.0, (tilt + 1.0) * 127.5)));
}

UartFeatures buildUartFeatures(BeatDetector& detector, bool isBeat) {
    UartFeatures f;
    f.volumeLow    = quantizeVolume(detector.getVolumeLow());
    f.volumeMid    = quantizeVolume(detector.getVolumeMid());
    f.volumeHigh   = quantizeVolume(detector.getVolumeHigh());
    f.spectralTilt = quantizeTilt(detector.getSpectralTilt());

    if (isBeat) {
        if (detector.isPeakLow())  f.beatFlags |= UartFeatures::BEAT_LOW;
        if (detector.isPeakMid())  f.beatFlags |= UartFeatures::BEAT_MID;
        if (detector.isPeakHigh()) f.beatFlags |= UartFeatures::BEAT_HIGH;
        f.beatFlags |= UartFeatures::BEAT_FUSED;
    }
    return f;
}

// ── Misc helpers ──────────────────────────────────────────────────────────────

bool button_state = false;
void checkButtonPressed() {
    if (pushButton.isPressed()) {
        if (button_state == false) {
            std::cout << "Button Pressed" << std::endl;
            uint8_t btn = UART_BUTTON_PRESSED;
            uart.send(&btn, 1);
        }
        button_state = true;
    } else {
        if (button_state == true) {
            std::cout << "Button Released" << std::endl;
        }
        button_state = false;
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

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return false;
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main() {
    if (!uart.initialize(serialConfig, ESP_UART_BAUD_RATE)) {
        std::cout << "Error setting up uart" << std::endl;
        return 1;
    }

    AudioBuffer buffer;

    UsbMicro usbMicro(microphone_device_id, SoundIoBackendAlsa);
    if (!initializeMicrophone(usbMicro)) {
        std::cout << "Error initializing Microphone" << std::endl;
        return 1;
    }

    // ── Shared state (protected by beat_ready_mutex) ──────────────────────────
    std::mutex beat_ready_mutex;
    std::condition_variable beat_ready_cv;
    bool beat_ready = false;
    std::chrono::steady_clock::time_point beat_detected_time;
    UartFeatures latest_features;

    // ── UART sender thread ────────────────────────────────────────────────────
    // Wakes immediately on a beat, or after 40 ms to send a volume-only update.
    std::thread beat_uart_thread([&]() {
        while (true) {
            UartFeatures to_send;
            bool beat_occurred;
            std::chrono::steady_clock::time_point detected_time_copy;

            {
                std::unique_lock<std::mutex> lock(beat_ready_mutex);
                beat_occurred = beat_ready_cv.wait_for(
                    lock,
                    std::chrono::milliseconds(40),
                    [&] { return beat_ready; }
                );
                to_send = latest_features;
                latest_features.beatFlags = 0; // consumed — don't re-send beat next timeout
                if (beat_occurred) {
                    detected_time_copy = beat_detected_time;
                    beat_ready = false;
                }
            }

            if (beat_occurred) {
                auto now = std::chrono::steady_clock::now();
                auto latency_us = std::chrono::duration_cast<std::chrono::microseconds>(
                    now - detected_time_copy).count();
                if (latency_us > 1000) {
                    std::cout << "[Latency] Beat detect → UART thread: " << latency_us << " µs" << std::endl;
                }
            }

            uart_send_features(to_send);
        }
    });

    BeatDetectionConfig config;
    config.thresholdRelLow  = 0.1;
    config.thresholdRelMid  = 0.1;
    config.thresholdRelHigh = 0.1;
    config.peakDetectionMaxBpm = 180.0;

    Downsampler downsampler(usbMicro.getSampleRate(), SIGNAL_DOWNSAMPLE_RATIO,
                            config.downsampleCutoffFrequency);
    AutomaticGainControl automaticGainControl(
        usbMicro.getSampleRate() / SIGNAL_DOWNSAMPLE_RATIO,
        config.automaticGainControlTargetLevel);
    jellED::SampleRecorder recorder(12000 * 5, "output.wav", 12000);

    std::cout << "Microphone sample rate: " << usbMicro.getSampleRate() << " Hz" << std::endl;
    std::cout << "Beat detection sample rate: "
              << (usbMicro.getSampleRate() / SIGNAL_DOWNSAMPLE_RATIO) << " Hz" << std::endl;

    BeatDetector* beatDetector = new BeatDetector(
        usbMicro.getSampleRate() / SIGNAL_DOWNSAMPLE_RATIO, config);

    usbMicro.drain_audio_buffer();

    std::cout << "Starting main audio processing loop..." << std::endl;

    int samplesRecordedTime      = 0;
    long samplesRecordedTimeTotal = 0;
    int totalSamplesRead         = 0;
    int readAudioTimeTotal       = 0;
    int flushCounter             = 0;
    int volumeUpdateCounter      = 0; // counts downsampled samples for periodic volume send

    while (true) {
        auto readAudioStart = std::chrono::steady_clock::now();
        bool gotAudio = usbMicro.read(&buffer);

        if (++flushCounter >= 750) {
            usbMicro.flushEvents();
            flushCounter = 0;
        }

        if (gotAudio) {
            auto readAudioEnd = std::chrono::steady_clock::now();
            uint32_t readAudioTimeUs = static_cast<uint32_t>(
                std::chrono::duration_cast<std::chrono::microseconds>(
                    readAudioEnd - readAudioStart).count());
            readAudioTimeTotal += readAudioTimeUs;
            totalSamplesRead += buffer.num_samples;
            if (totalSamplesRead >= 48000) {
                std::cout << "Read audio took: " << readAudioTimeTotal << " µs" << std::endl;
                readAudioTimeTotal = 0;
                totalSamplesRead   = 0;
            }
        }

        auto start = std::chrono::steady_clock::now();

        if (gotAudio) {
            jellED::AudioBuffer downsampledBuffer;
            downsampler.downsample(buffer, downsampledBuffer);

            for (int i = 0; i < downsampledBuffer.num_samples; i++) {
                samplesRecordedTime++;
                double sample = downsampledBuffer.buffer[i];

                if (recorder.addSample(sample)) {
                    recorder.setEnabled(false);
                }

                bool isBeat = beatDetector->is_beat(sample);

                if (isBeat) {
                    // Beat detected: build full feature packet and wake UART thread.
                    std::cout << "Beat!" << std::endl;
                    {
                        std::lock_guard<std::mutex> lock(beat_ready_mutex);
                        latest_features      = buildUartFeatures(*beatDetector, true);
                        beat_detected_time   = std::chrono::steady_clock::now();
                        beat_ready           = true;
                    }
                    beat_ready_cv.notify_one();
                }

                // Periodic volume update: keep latest_features current so the
                // 40 ms timeout always sends fresh volume data to the ESP.
                if (++volumeUpdateCounter >= VOLUME_UPDATE_INTERVAL) {
                    volumeUpdateCounter = 0;
                    std::lock_guard<std::mutex> lock(beat_ready_mutex);
                    UartFeatures vol = buildUartFeatures(*beatDetector, false);
                    latest_features.volumeLow    = vol.volumeLow;
                    latest_features.volumeMid    = vol.volumeMid;
                    latest_features.volumeHigh   = vol.volumeHigh;
                    latest_features.spectralTilt = vol.spectralTilt;
                    // beatFlags intentionally not overwritten — a pending beat stays pending
                }
            }
        }

        auto end = std::chrono::steady_clock::now();
        uint32_t beatDetectionTimeUs = static_cast<uint32_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        samplesRecordedTimeTotal += beatDetectionTimeUs;

        if (samplesRecordedTime >= 6000) {
            std::cout << "Beat detection time: "
                      << samplesRecordedTimeTotal / samplesRecordedTime << " µs/sample" << std::endl;
            samplesRecordedTime      = 0;
            samplesRecordedTimeTotal = 0;
        }
    }
}
