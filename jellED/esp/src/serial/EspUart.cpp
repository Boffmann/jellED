#include "EspUart.h"
#include "esplogger.h"
#include <cstring>

namespace jellED {
namespace serial {

EspUart::EspUart() 
    : uartPort(UART_NUM_0), txPin(-1), rxPin(-1), initialized(false) {
}

EspUart::~EspUart() {
    if (initialized) {
        close();
    }
}

bool EspUart::initialize(const SerialConfig& config) {
    UartConfig uartConfig;
    uartConfig.baudRate = config.baudRate;
    uartConfig.dataBits = config.dataBits;
    uartConfig.stopBits = config.stopBits;
    uartConfig.parity = config.parity;
    uartConfig.flowControl = config.flowControl;
    uartConfig.timeoutMs = config.timeoutMs;
    uartConfig.portName = "UART" + std::to_string(uartPort);
    uartConfig.uartNumber = uartPort;
    
    return initialize(uartConfig);
}

bool EspUart::initialize(const UartConfig& uartConfig) {
    if (initialized) {
        close();
    }
    
    config = uartConfig;
    
    // Configure UART parameters
    uart_config_t uartConfig_esp = {};
    uartConfig_esp.baud_rate = getEspBaudRate(config.baudRate);
    uartConfig_esp.data_bits = getEspDataBits(config.dataBits);
    uartConfig_esp.stop_bits = getEspStopBits(config.stopBits);
    uartConfig_esp.parity = getEspParity(config.parity);
    uartConfig_esp.flow_ctrl = config.flowControl ? UART_HW_FLOWCTRL_CTS_RTS : UART_HW_FLOWCTRL_DISABLE;
    uartConfig_esp.source_clk = UART_SCLK_APB;
    
    // Install UART driver
    esp_err_t ret = uart_driver_install(uartPort, 1024, 1024, 0, NULL, 0);
    if (ret != ESP_OK) {
        return false;
    }
    
    // Configure UART parameters
    ret = uart_param_config(uartPort, &uartConfig_esp);
    if (ret != ESP_OK) {
        uart_driver_delete(uartPort);
        return false;
    }
    
    // Set UART pins if specified
    if (txPin >= 0 && rxPin >= 0) {
        ret = uart_set_pin(uartPort, txPin, rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        if (ret != ESP_OK) {
            uart_driver_delete(uartPort);
            return false;
        }
    }
    
    // Set timeout
    uart_set_rx_timeout(uartPort, config.timeoutMs / 10); // Convert to UART timeout units
    
    initialized = true;
    return true;
}

bool EspUart::isAvailable() const {
    return initialized;
}

int EspUart::send(const uint8_t* data, size_t length) {
    if (!initialized) {
        return -1;
    }
    
    int written = uart_write_bytes(uartPort, data, length);
    return written;
}

int EspUart::receive(uint8_t* buffer, size_t maxLength) {
    if (!initialized) {
        return -1;
    }
    
    int read = uart_read_bytes(uartPort, buffer, maxLength, pdMS_TO_TICKS(config.timeoutMs));
    return read;
}

int EspUart::available() const {
    if (!initialized) {
        return 0;
    }
    
    size_t length = 0;
    uart_get_buffered_data_len(uartPort, &length);
    return static_cast<int>(length);
}

void EspUart::flush() {
    if (initialized) {
        uart_flush(uartPort);
    }
}

void EspUart::close() {
    if (initialized) {
        uart_driver_delete(uartPort);
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
        esp_err_t ret = uart_set_pin(uartPort, txPin, rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        return ret == ESP_OK;
    }
    
    return true;
}

std::string EspUart::getPortName() const {
    return config.portName;
}

int EspUart::getUartNumber() const {
    return config.uartNumber;
}

bool EspUart::isLoopbackEnabled() const {
    return config.enableLoopback;
}

void EspUart::setUartPort(uart_port_t port) {
    if (!initialized) {
        uartPort = port;
        config.uartNumber = port;
        config.portName = "UART" + std::to_string(port);
    }
}

uart_port_t EspUart::getUartPort() const {
    return uartPort;
}

uart_baud_rate_t EspUart::getEspBaudRate(uint32_t baudRate) const {
    switch (baudRate) {
        case 1200: return (uart_baud_rate_t)1200;
        case 2400: return (uart_baud_rate_t)2400;
        case 4800: return (uart_baud_rate_t)4800;
        case 9600: return (uart_baud_rate_t)9600;
        case 19200: return (uart_baud_rate_t)19200;
        case 38400: return (uart_baud_rate_t)38400;
        case 57600: return (uart_baud_rate_t)57600;
        case 115200: return (uart_baud_rate_t)115200;
        case 230400: return (uart_baud_rate_t)230400;
        case 460800: return (uart_baud_rate_t)460800;
        case 921600: return (uart_baud_rate_t)921600;
        default: return (uart_baud_rate_t)115200;
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

} // namespace serial
} // namespace jellED 