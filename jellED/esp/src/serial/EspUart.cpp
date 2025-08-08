#include "EspUart.h"
#include <Arduino.h>
#include "esplogger.h"
#include <cstring>

namespace jellED {

EspUart::EspUart(const std::string portName, const int portNumber, const int txPin, const int rxPin) 
    : portName(portName), portNumber(portNumber), txPin(txPin), rxPin(rxPin), initialized(false), baudRate(115200) {
}

EspUart::~EspUart() {
    if (initialized) {
        close();
    }
}

bool EspUart::initialize(const SerialConfig& config, const uint32_t baudRate) {
    Serial.println("Initializing Esp Uart");
    if (initialized) {
        close();
    }
    
    this->config = config;
    this->baudRate = baudRate;
    
    // Validate UART number
    if (this->portNumber < 0 || this->portNumber > UART_NUM_2) {
        Serial.println("Invalid UART number");
        return false;
    }
    
    // Install UART driver with reasonable buffer sizes
    Serial.println("Installing UART Driver");
    esp_err_t ret = uart_driver_install(this->portNumber, 1024, 1024, 10, &this->uart_queue, 0);
    if (ret != ESP_OK) {
        Serial.println("Installing UART Driver - Failure");
        return false;
    }
    Serial.println("Installing UART Driver - Success");
    
    // Configure UART parameters
    uart_config_t uartConfig_esp = {};
    uartConfig_esp.baud_rate = baudRate;
    uartConfig_esp.data_bits = getEspDataBits(config.dataBits);
    uartConfig_esp.stop_bits = getEspStopBits(config.stopBits);
    uartConfig_esp.parity = getEspParity(config.parity);
    uartConfig_esp.flow_ctrl = config.flowControl ? UART_HW_FLOWCTRL_CTS_RTS : UART_HW_FLOWCTRL_DISABLE;
    
    // Configure UART parameters
    ret = uart_param_config(this->portNumber, &uartConfig_esp);
    if (ret != ESP_OK) {
        uart_driver_delete(this->portNumber);
        Serial.println("UART parameter configuration failed");
        return false;
    }
    
    // Set UART pins if specified
    if (txPin >= 0 && rxPin >= 0) {
        ret = uart_set_pin(this->portNumber, txPin, rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        if (ret != ESP_OK) {
            uart_driver_delete(this->portNumber);
            Serial.println("UART pin configuration failed");
            return false;
        }
    } else {
        uart_driver_delete(this->portNumber);
        Serial.println("UART parameter configuration failed");
        return false;
    }
    
    // Set timeout (convert ms to UART timeout units, minimum 1)
    uint32_t timeout_units = (config.timeoutMs + 9) / 10; // Round up division
    if (timeout_units < 1) timeout_units = 1;
    uart_set_rx_timeout(this->portNumber, timeout_units);
    
    initialized = true;
    return true;
}

bool EspUart::isInitialized() const {
    return initialized;
}

// TODO Proper error handling
int EspUart::send(const uint8_t* data, size_t length) {
    if (!initialized || !data) {
        return -1;
    }
    
    if (length == 0) {
        return -1;
    }
    
    int written = uart_write_bytes(this->portNumber, data, length);
    
    if (written > 0) {
        // Wait for transmission to complete with a reasonable timeout
        // Use a timeout based on the data size and baud rate
        uint32_t timeout_ms = (length * 10 * 1000) / baudRate + 100; // Add 100ms buffer
        if (timeout_ms < 100) timeout_ms = 100; // Minimum 100ms
        if (timeout_ms > 5000) timeout_ms = 5000; // Maximum 5 seconds
        
        esp_err_t wait_result = uart_wait_tx_done(this->portNumber, pdMS_TO_TICKS(timeout_ms));
        if (wait_result != ESP_OK) {
            Serial.println("Warning: UART transmission timeout");
        }
    }
    
    return written;
}

int EspUart::send(const std::string& data) {
    if (!initialized) {
        return -1;
    }

    if (data.empty()) {
        return -1;
    }

    // Use the size() method instead of strlen for better performance
    // and to handle strings with embedded null characters correctly
    int written = uart_write_bytes(this->portNumber, data.c_str(), data.size());
    
    if (written > 0) {
        // Wait for transmission to complete with a reasonable timeout
        // Use a timeout based on the data size and baud rate
        uint32_t timeout_ms = (data.size() * 10 * 1000) / baudRate + 100; // Add 100ms buffer
        if (timeout_ms < 100) timeout_ms = 100; // Minimum 100ms
        if (timeout_ms > 5000) timeout_ms = 5000; // Maximum 5 seconds
        
        esp_err_t wait_result = uart_wait_tx_done(this->portNumber, pdMS_TO_TICKS(timeout_ms));
        if (wait_result != ESP_OK) {
            Serial.println("Warning: UART transmission timeout");
        }
    }
    
    return written;
}

int EspUart::receive(uint8_t* buffer, size_t maxLength) {
    if (!initialized || !buffer) {
        return -1;
    }
    
    if (maxLength == 0) {
        return -1;
    }
    
    int read = uart_read_bytes(this->portNumber, buffer, maxLength, pdMS_TO_TICKS(config.timeoutMs));
    return read;
}

int EspUart::receive(std::string& data, size_t maxLength) {
    if (!initialized) {
        return -1;
    }

    // Validate maxLength to prevent excessive memory allocation
    if (maxLength == 0 || maxLength > 8192) { // Reasonable upper limit
        return -1;
    }

    // Use stack allocation for small buffers, heap for large ones
    const size_t STACK_THRESHOLD = 256;
    char* buffer = nullptr;
    bool use_heap = maxLength > STACK_THRESHOLD;
    
    if (use_heap) {
        buffer = static_cast<char*>(malloc(maxLength));
        if (!buffer) {
            return -1; // Memory allocation failed
        }
    } else {
        buffer = static_cast<char*>(alloca(maxLength));
    }
    
    // Read data from UART
    int read = uart_read_bytes(this->portNumber, buffer, maxLength, pdMS_TO_TICKS(config.timeoutMs));
    
    if (read > 0) {
        // Only copy the actual bytes read, not the entire buffer
        data.assign(buffer, read);
    } else {
        data.clear();
    }
    
    // Free heap memory if used
    if (use_heap) {
        free(buffer);
    }
    
    return read;
}

int EspUart::available() const {
    if (!initialized) {
        return 0;
    }
    
    size_t length = 0;
    uart_get_buffered_data_len(this->portNumber, &length);
    return static_cast<int>(length);
}

void EspUart::flush() {
    if (initialized) {
        uart_flush(this->portNumber);
    }
}

void EspUart::close() {
    if (initialized) {
        uart_driver_delete(this->portNumber);
        initialized = false;
    }
}

uart_word_length_t EspUart::getEspDataBits(uint8_t dataBits) const {
    switch (dataBits) {
        case 5: return UART_DATA_5_BITS;
        case 6: return UART_DATA_6_BITS;
        case 7: return UART_DATA_7_BITS;
        case 8: return UART_DATA_8_BITS;
        default: return UART_DATA_8_BITS;
    }
}

uart_stop_bits_t EspUart::getEspStopBits(uint8_t stopBits) const {
    switch (stopBits) {
        case 1: return UART_STOP_BITS_1;
        case 2: return UART_STOP_BITS_2;
        default: return UART_STOP_BITS_1;
    }
}

uart_parity_t EspUart::getEspParity(bool parity) const {
    return parity ? UART_PARITY_EVEN : UART_PARITY_DISABLE;
}

} // namespace jellED 