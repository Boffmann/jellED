#include "pUtils/raspi/include/raspiutils.h"
#include "serial/raspi/RaspiUart.h"
#include "pushButton/pushbutton.h"
#include "sound/raspi/usbMicro.h"
#include "beatdetection.h"
#include "uartProtocol.h"

#include <iostream>
#include <thread>
#include <fstream>

using namespace jellED;

constexpr uint32_t ESP_UART_BAUD_RATE = 115200;
constexpr bool MODE_SEND = true;
constexpr uint8_t PUSH_BUTTON_PIN = 4;

std::string microphone_device_id = "hw:CARD=Device,DEV=0";

RaspiPlatformUtils raspiUtils;
SerialConfig serialConfig;
RaspiUart uart("/dev/ttyS0", 0);

PushButton pushButton(raspiUtils, PUSH_BUTTON_PIN);

void uart_send_int(const uint8_t dataToWrite) {
	if (uart.send(&dataToWrite, 1) == -1) {
		std::cout << "Failed to write to uart" << std::endl;
	} else {
		std::cout << "Written to uart: " << unsigned(dataToWrite) << std::endl;
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

void write_samples_to_file(const std::vector<float>& samples, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing" << std::endl;
        return;
    }

    // Write each sample on a separate line
    for (float sample : samples) {
        file << sample << std::endl;
    }

    file.close();
    std::cout << "Wrote " << samples.size() << " samples to " << filename << std::endl;
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

int main () {
    if (!uart.initialize(serialConfig, ESP_UART_BAUD_RATE)) {
    	std::cout << "Error setting up uart" << std::endl;
		return 1;
    }

    AudioBuffer buffer;

    // UsbMicro::print_available_input_devices();
    UsbMicro usbMicro(microphone_device_id);
    if (!initializeMicrophone(usbMicro)) {
        std::cout << "Error initializing Microphone" << std::endl;
        return 1;
    }

    std::thread button_thread([]() {
        while(true) {
            checkButtonPressed();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    });

    BeatDetector beatDetection(raspiUtils, usbMicro.getSampleRate());
    while(true) {
        if (usbMicro.read(&buffer)) {
            for (int i = 0; i < buffer.num_samples; i++) {
                // std::cout << "Feeding sample: " << buffer.buffer[i] << std::endl;
                if (beatDetection.is_beat(buffer.buffer[i])) {
                    uart_send_int(UART_BEAT_DETECTED);
                }
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}
