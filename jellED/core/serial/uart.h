#ifndef __JELLED_UART_H__
#define __JELLED_UART_H__

#include "ISerial.h"
#include <string>

namespace jellED {

/**
 * @brief UART-specific configuration
 */
struct UartConfig : public SerialConfig {
    std::string portName;      // Port name (e.g., "/dev/ttyUSB0", "COM1", "UART0")
    int uartNumber;            // UART number (0, 1, 2, etc.)
    bool invertTx;             // Invert TX signal
    bool invertRx;             // Invert RX signal
    bool enableLoopback;       // Enable loopback mode for testing
    
    UartConfig() : uartNumber(0), invertTx(false), invertRx(false), enableLoopback(false) {}
    UartConfig(const std::string& port, int uart = 0, uint32_t baud = 115200)
        : SerialConfig(baud), portName(port), uartNumber(uart), 
          invertTx(false), invertRx(false), enableLoopback(false) {}
};

/**
 * @brief UART communication interface
 */
class IUart : public ISerial {
public:
    virtual ~IUart() = default;
    
    /**
     * @brief Initialize UART with specific configuration
     * @param config UART configuration
     * @return true if initialization successful
     */
    virtual bool initialize(const UartConfig& config) = 0;
    
    /**
     * @brief Get current UART configuration
     * @return Current UART configuration
     */
    virtual UartConfig getConfig() const = 0;
    
    /**
     * @brief Set UART pins (platform-specific)
     * @param txPin TX pin number
     * @param rxPin RX pin number
     * @return true if pin configuration successful
     */
    virtual bool setPins(int txPin, int rxPin) = 0;
    
    /**
     * @brief Get UART port name
     * @return Port name string
     */
    virtual std::string getPortName() const = 0;
    
    /**
     * @brief Get UART number
     * @return UART number
     */
    virtual int getUartNumber() const = 0;
    
    /**
     * @brief Check if UART is in loopback mode
     * @return true if loopback mode is enabled
     */
    virtual bool isLoopbackEnabled() const = 0;
};

} // namespace jellED

#endif // __JELLED_UART_H__
