#include "EspUart.h"
#include <Arduino.h>
#include "esplogger.h"
#include <cstring>

namespace jellED {

EspUart::EspUart() 
    : txPin(-1), rxPin(-1), initialized(false) {
}

EspUart::~EspUart() {
    if (initialized) {
        close();
    }
}

bool EspUart::initialize(const UartConfig& uartConfig) {
    Serial.println("Initializing Esp Uart");
    if (initialized) {
        close();
    }
    
    this->config = uartConfig;
    
    // Install UART driver
    Serial.println("Installing UART Driver");
    esp_err_t ret = uart_driver_install(this->config.uartNumber, 1024, 1024, 10, &this->uart_queue, 0);
    if (ret != ESP_OK) {
        Serial.println("Installing UART Driver - Failure");
        return false;
    }
    Serial.println("Installing UART Driver - Success");
    
    // Configure UART parameters
    uart_config_t uartConfig_esp = {};
    uartConfig_esp.baud_rate = config.baudRate;
    uartConfig_esp.data_bits = getEspDataBits(config.dataBits);
    uartConfig_esp.stop_bits = getEspStopBits(config.stopBits);
    uartConfig_esp.parity = getEspParity(config.parity);
    uartConfig_esp.flow_ctrl = config.flowControl ? UART_HW_FLOWCTRL_CTS_RTS : UART_HW_FLOWCTRL_DISABLE;
    
    // Configure UART parameters
    ret = uart_param_config(this->config.uartNumber, &uartConfig_esp);
    if (ret != ESP_OK) {
        uart_driver_delete(this->config.uartNumber);
        return false;
    }
    
    // Set UART pins if specified
    if (txPin >= 0 && rxPin >= 0) {
        ret = uart_set_pin(this->config.uartNumber, txPin, rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        if (ret != ESP_OK) {
            uart_driver_delete(this->config.uartNumber);
            return false;
        }
    }
    
    // Set timeout
    uart_set_rx_timeout(this->config.uartNumber, config.timeoutMs / 10); // Convert to UART timeout units
    
    initialized = true;
    return true;
}

bool EspUart::isInitialized() const {
    return initialized;
}

int EspUart::send(const uint8_t* data, size_t length) {
    if (!initialized) {
        return -1;
    }
    
    int written = uart_write_bytes(this->config.uartNumber, data, length);
    return written;
}

int EspUart::send(const std::string& data) {
    if (!initialized) {
        return -1;
    }

    int written = uart_write_bytes(this->config.uartNumber, (const char*) data.c_str(), strlen(data.c_str()));
    return written;
}

int EspUart::receive(uint8_t* buffer, size_t maxLength) {
    if (!initialized) {
        return -1;
    }
    
    int read = uart_read_bytes(this->config.uartNumber, buffer, maxLength, pdMS_TO_TICKS(config.timeoutMs));
    return read;
}

int EspUart::receive(std::string& data, size_t maxLength) {
    if (!initialized) {
        return -1;
    }

    char* buffer;
    
    int read = uart_read_bytes(this->config.uartNumber, buffer, maxLength, pdMS_TO_TICKS(config.timeoutMs));
    data = std::string(buffer);
    return read;
}

int EspUart::available() const {
    if (!initialized) {
        return 0;
    }
    
    size_t length = 0;
    uart_get_buffered_data_len(this->config.uartNumber, &length);
    return static_cast<int>(length);
}

void EspUart::flush() {
    if (initialized) {
        uart_flush(this->config.uartNumber);
    }
}

void EspUart::close() {
    if (initialized) {
        uart_driver_delete(this->config.uartNumber);
        initialized = false;
    }
}

UartConfig EspUart::getConfig() const {
    return config;
}

bool EspUart::setPins(int tx, int rx) {
    txPin = tx;
    rxPin = rx;
    
    if (initialized) {
        // Reconfigure pins if already initialized
        esp_err_t ret = uart_set_pin(this->config.uartNumber, txPin, rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        return ret == ESP_OK;
    }
    
    return true;
}

std::string EspUart::getPortName() const {
    return config.portName;
}

int EspUart::getUartNumber() const {
    return this->config.uartNumber;
}

bool EspUart::isLoopbackEnabled() const {
    return config.enableLoopback;
}

void EspUart::setUartPort(uart_port_t port) {
    if (!initialized) {
        config.uartNumber = port;
        config.portName = "UART" + std::to_string(port);
    }
}

uart_port_t EspUart::getUartPort() const {
    return this->config.uartNumber;
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