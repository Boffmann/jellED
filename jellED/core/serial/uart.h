#ifndef __JELLED_UART_H__
#define __JELLED_UART_H__

#include "ISerial.h"
#include <string>

namespace jellED {

/**
 * @brief UART communication interface
 */
class IUart : public ISerial {
public:
    virtual ~IUart() = default;
    
    /**
     * @brief Initialize UART with specific configuration
     * @param config UART configuration
     * @param baudRate UART baud rate
     * @return true if initialization successful
     */
    virtual bool initialize(const SerialConfig& config, const uint32_t baudRate) = 0;
};

} // namespace jellED

#endif // __JELLED_UART_H__
