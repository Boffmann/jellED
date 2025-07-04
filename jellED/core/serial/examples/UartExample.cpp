#include "uart.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace jellED::serial;

/**
 * @brief Example demonstrating UART communication between RaspberryPi and ESP32
 */
class UartExample {
private:
    std::unique_ptr<IUart> uart;
    
public:
    UartExample() = default;
    
    /**
     * @brief Initialize UART with default settings
     * @param portName Port name (platform-specific)
     * @param uartNumber UART number
     * @return true if initialization successful
     */
    bool initialize(const std::string& portName = "", int uartNumber = 0) {
        UartConfig config;
        config.portName = portName.empty() ? "UART" + std::to_string(uartNumber) : portName;
        config.uartNumber = uartNumber;
        config.baudRate = 115200;
        config.dataBits = 8;
        config.stopBits = 1;
        config.parity = false;
        config.flowControl = false;
        config.timeoutMs = 1000;
        
        // Note: In a real application, you would create the appropriate platform-specific UART
        // For example:
        // #ifdef ESP32
        //     uart = std::make_unique<EspUart>();
        // #elif RASPBERRY_PI
        //     uart = std::make_unique<RaspiUart>();
        // #endif
        
        return uart && uart->initialize(config);
    }
    
    /**
     * @brief Send a message over UART
     * @param message Message to send
     * @return Number of bytes sent
     */
    int sendMessage(const std::string& message) {
        if (!uart || !uart->isAvailable()) {
            std::cerr << "UART not available" << std::endl;
            return -1;
        }
        
        std::cout << "Sending: " << message << std::endl;
        int sent = uart->send(message);
        if (sent > 0) {
            std::cout << "Sent " << sent << " bytes" << std::endl;
        } else {
            std::cerr << "Failed to send message" << std::endl;
        }
        return sent;
    }
    
    /**
     * @brief Receive a message from UART
     * @param maxLength Maximum message length
     * @return Received message
     */
    std::string receiveMessage(size_t maxLength = 256) {
        if (!uart || !uart->isAvailable()) {
            std::cerr << "UART not available" << std::endl;
            return "";
        }
        
        std::string message;
        int received = uart->receive(message, maxLength);
        if (received > 0) {
            std::cout << "Received: " << message << " (" << received << " bytes)" << std::endl;
        } else if (received == 0) {
            std::cout << "No data available" << std::endl;
        } else {
            std::cerr << "Error receiving message" << std::endl;
        }
        return message;
    }
    
    /**
     * @brief Check for available data
     * @return Number of bytes available
     */
    int checkAvailable() {
        if (!uart) {
            return 0;
        }
        return uart->available();
    }
    
    /**
     * @brief Run a simple echo test
     */
    void runEchoTest() {
        std::cout << "Starting UART echo test..." << std::endl;
        
        // Send test message
        std::string testMessage = "Hello from UART!";
        if (sendMessage(testMessage) > 0) {
            // Wait a bit for response
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Try to receive response
            std::string response = receiveMessage();
            if (!response.empty()) {
                std::cout << "Echo test successful!" << std::endl;
            } else {
                std::cout << "No echo received" << std::endl;
            }
        }
    }
    
    /**
     * @brief Run a continuous communication loop
     */
    void runCommunicationLoop() {
        std::cout << "Starting UART communication loop..." << std::endl;
        std::cout << "Press Ctrl+C to stop" << std::endl;
        
        while (true) {
            // Check for incoming data
            if (checkAvailable() > 0) {
                std::string message = receiveMessage();
                if (!message.empty()) {
                    // Echo back the received message
                    sendMessage("Echo: " + message);
                }
            }
            
            // Small delay to prevent busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
};

/**
 * @brief Example usage for RaspberryPi
 */
void raspberryPiExample() {
    std::cout << "RaspberryPi UART Example" << std::endl;
    
    UartExample uart;
    
    // Initialize with default RaspberryPi UART port
    if (uart.initialize("/dev/ttyAMA0", 0)) {
        std::cout << "UART initialized successfully" << std::endl;
        
        // Send a message to ESP32
        uart.sendMessage("Hello from RaspberryPi!");
        
        // Wait for response
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Receive response
        uart.receiveMessage();
        
    } else {
        std::cerr << "Failed to initialize UART" << std::endl;
    }
}

/**
 * @brief Example usage for ESP32
 */
void esp32Example() {
    std::cout << "ESP32 UART Example" << std::endl;
    
    UartExample uart;
    
    // Initialize with UART0
    if (uart.initialize("UART0", 0)) {
        std::cout << "UART initialized successfully" << std::endl;
        
        // Set UART pins (TX: GPIO17, RX: GPIO16)
        // Note: This would be done through the platform-specific implementation
        
        // Send a message to RaspberryPi
        uart.sendMessage("Hello from ESP32!");
        
        // Wait for response
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Receive response
        uart.receiveMessage();
        
    } else {
        std::cerr << "Failed to initialize UART" << std::endl;
    }
}

int main() {
    std::cout << "jellED UART Communication Example" << std::endl;
    std::cout << "=================================" << std::endl;
    
    // This example shows the interface usage
    // In a real application, you would compile with the appropriate platform
    // and use the platform-specific UART implementation
    
    std::cout << "This is a demonstration of the UART interface." << std::endl;
    std::cout << "To use with actual hardware:" << std::endl;
    std::cout << "1. Include the appropriate platform-specific header" << std::endl;
    std::cout << "2. Create an instance of EspUart or RaspiUart" << std::endl;
    std::cout << "3. Configure pins and initialize" << std::endl;
    std::cout << "4. Use the ISerial interface methods" << std::endl;
    
    return 0;
} 