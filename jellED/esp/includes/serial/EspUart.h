#ifndef __JELLED_ESP_UART_H__
#define __JELLED_ESP_UART_H__

#include "uart.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include <memory>

namespace jellED {
namespace serial {

/**
 * @brief ESP32-specific UART implementation
 */
class EspUart : public IUart {
private:
    uart_port_t uartPort;
    int txPin;
    int rxPin;
    bool initialized;
    UartConfig config;
    
    // Convert baud rate to ESP32 format
    uart_baud_rate_t getEspBaudRate(uint32_t baudRate) const;
    
    // Convert data bits to ESP32 format
    uart_word_length_t getEspDataBits(uint8_t dataBits) const;
    
    // Convert stop bits to ESP32 format
    uart_stop_bits_t getEspStopBits(uint8_t stopBits) const;
    
    // Convert parity to ESP32 format
    uart_parity_t getEspParity(bool parity) const;
    
public:
    EspUart();
    virtual ~EspUart();
    
    // ISerial interface implementation
    bool initialize(const SerialConfig& config) override;
    bool initialize(const UartConfig& config) override;
    bool isAvailable() const override;
    int send(const uint8_t* data, size_t length) override;
    int receive(uint8_t* buffer, size_t maxLength) override;
    int available() const override;
    void flush() override;
    void close() override;
    
    // IUart interface implementation
    UartConfig getConfig() const override;
    bool setPins(int txPin, int rxPin) override;
    std::string getPortName() const override;
    int getUartNumber() const override;
    bool isLoopbackEnabled() const override;
    
    /**
     * @brief Set UART port number (0, 1, 2)
     * @param port UART port number
     */
    void setUartPort(uart_port_t port);
    
    /**
     * @brief Get UART port number
     * @return UART port number
     */
    uart_port_t getUartPort() const;
};

} // namespace serial
} // namespace jellED

#endif // __JELLED_ESP_UART_H__ 