#ifndef __JELLED_ESP_UART_H__
#define __JELLED_ESP_UART_H__

#include "uart.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include <memory>

namespace jellED {

/**
 * @brief ESP32-specific UART implementation
 */
class EspUart : public IUart {
private:
    std::string portName;
    int portNumber;
    int txPin;
    int rxPin;
    bool initialized;
    SerialConfig config;
    uint32_t baudRate;
    QueueHandle_t uart_queue;
    
    // Convert data bits to ESP32 format
    uart_word_length_t getEspDataBits(uint8_t dataBits) const;
    
    // Convert stop bits to ESP32 format
    uart_stop_bits_t getEspStopBits(uint8_t stopBits) const;
    
    // Convert parity to ESP32 format
    uart_parity_t getEspParity(bool parity) const;
    
public:
    EspUart(const std::string portName, const int portNumber, const int txPin, const int rxPin);
    virtual ~EspUart();
    
    // ISerial interface implementation
    bool initialize(const SerialConfig& config, const uint32_t baudRate) override;
    bool isInitialized() const override;
    int send(const uint8_t* data, size_t length) override;
    int send(const std::string& data) override;
    int receive(uint8_t* buffer, size_t maxLength) override;
    int receive(std::string& data, size_t maxLength) override;
    int available() const override;
    void flush() override;
    void close() override;
};

} // namespace jellED

#endif // __JELLED_ESP_UART_H__ 
