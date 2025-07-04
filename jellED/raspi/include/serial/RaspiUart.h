#ifndef __JELLED_RASPI_UART_H__
#define __JELLED_RASPI_UART_H__

#include "uart.h"
#include <string>
#include <termios.h>

namespace jellED {
namespace serial {

/**
 * @brief RaspberryPi-specific UART implementation
 */
class RaspiUart : public IUart {
private:
    int fileDescriptor;
    std::string portName;
    int uartNumber;
    bool initialized;
    UartConfig config;
    struct termios originalTios;
    struct termios currentTios;
    
    // Convert baud rate to Linux format
    speed_t getLinuxBaudRate(uint32_t baudRate) const;
    
    // Configure termios structure
    bool configureTermios();
    
    // Restore original terminal settings
    void restoreTermios();
    
public:
    RaspiUart();
    virtual ~RaspiUart();
    
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
     * @brief Set UART port name (e.g., "/dev/ttyAMA0", "/dev/ttyUSB0")
     * @param port Port name
     */
    void setPortName(const std::string& port);
    
    /**
     * @brief Get file descriptor
     * @return File descriptor, -1 if not open
     */
    int getFileDescriptor() const;
    
    /**
     * @brief Check if port is open
     * @return true if port is open
     */
    bool isOpen() const;
};

} // namespace serial
} // namespace jellED

#endif // __JELLED_RASPI_UART_H__ 