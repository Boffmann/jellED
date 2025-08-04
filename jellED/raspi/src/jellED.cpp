#include "utils/raspiutils.h"
#include "serial/RaspiUart.h"

#include <iostream>
#include <thread>

int main () {
    jellED::UartConfig uartConfig("/dev/ttyS0", 0);
    jellED::RaspiUart uart;
    if (!uart.initialize(uartConfig)) {
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
