#include "utils/raspiutils.h"
#include "serial/RaspiUart.h"

#include <iostream>
#include <thread>

constexpr uint32_t ESP_UART_BAUD_RATE = 115200;
constexpr bool MODE_SEND = true;

jellED::SerialConfig serialConfig;
jellED::RaspiUart uart("/dev/ttyS0", 0);

static uint8_t dataToWrite = 0;
void uart_send_int() {
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

std::string helloToSend = "Hello";
void uart_send_string() {
	if (uart.send(helloToSend) == -1) {
		std::cout << "Failed to write to uart" << std::endl;
	} else {
		std::cout << "Written to uart: " << helloToSend << std::endl;
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
}

uint8_t receiveBuffer = 0;
int maxLength = 8;
void uart_read_int() {
	int available = uart.available();
	if (available > 0) {
		int received = uart.receive(&receiveBuffer, std::min(available, maxLength));
		if (received > 0) {
			std::cout << "Received: " << unsigned(receiveBuffer) << " (" << received << " bytes)" << std::endl;
		} else if (received == 0) {
			std::cout << "No data available" << std::endl;
		} else {
			std::cout << "Error receiving message" << std::endl;
		}
      uart.flush();
	}
}

std::string receivedBuffer_string;
void uart_read_string() {
	int available = uart.available();
	if (available > 0) {
		int received = uart.receive(receivedBuffer_string, std::min(available, maxLength));
		if (received > 0) {
			std::cout << "Received: " << receivedBuffer_string << " (" << received << " bytes)" << std::endl;
		} else if (received == 0) {
			std::cout << "No data available" << std::endl;
		} else {
			std::cout << "Error receiving message" << std::endl;
		}
      uart.flush();
	}
}

int main () {
    if (!uart.initialize(serialConfig, ESP_UART_BAUD_RATE)) {
    	std::cout << "Error setting up uart" << std::endl;
		return 1;
    }
    

    while(true) {
		if (MODE_SEND) {
			uart_send_string();
		} else {
			uart_read_string();;
		}
    }
}
