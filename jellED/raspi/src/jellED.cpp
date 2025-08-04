#include "utils/raspiutils.h"
#include "serial/RaspiUart.h"

#include <iostream>
#include <thread>

constexpr uint32_t ESP_UART_BAUD_RATE = 115200;

int main () {
	jellED::SerialConfig serialConfig;
    jellED::RaspiUart uart("/dev/ttyS0", 0);
    if (!uart.initialize(serialConfig, ESP_UART_BAUD_RATE)) {
    	std::cout << "Error setting up uart" << std::endl;
		return 1;
    }
    uint8_t dataToWrite = 0;

    while(true) {
		dataToWrite = dataToWrite + 1;
		if (dataToWrite > 255) {
			dataToWrite = 0;
		}
        if (uart.send(&dataToWrite, 1) == -1) {
	  		std::cout << "Failed to write to uart" << std::endl;
		} else {
	  		std::cout << "Written to uart: " << unsigned(dataToWrite) << std::endl;
		}
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
}
