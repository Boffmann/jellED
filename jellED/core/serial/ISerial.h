#ifndef __JELLED_I_SERIAL_H__
#define __JELLED_I_SERIAL_H__

#include <string>
#include <vector>
#include <cstdint>

namespace jellED {

/**
 * @brief Serial communication configuration
 */
struct SerialConfig {
    uint8_t dataBits = 8;
    uint8_t stopBits = 1;
    bool parity = false;
    bool flowControl = false;
    uint32_t timeoutMs = 1000;

    SerialConfig(uint8_t data = 8, uint8_t stop = 1, 
                 bool par = false, bool flow = false, uint32_t timeout = 1000)
        : dataBits(data), stopBits(stop), 
          parity(par), flowControl(flow), timeoutMs(timeout) {}
};

/**
 * @brief Core serial communication interface
 */
class ISerial {
public:
    virtual ~ISerial() = default;
    
    /**
     * @brief Check if serial connection is initialized
     * @return true if connection is ready
     */
    virtual bool isInitialized() const = 0;
    
    /**
     * @brief Send data over serial connection
     * @param data Data to send
     * @param length Length of data
     * @return Number of bytes sent, -1 on error
     */
    virtual int send(const uint8_t* data, size_t length) = 0;
    
    /**
     * @brief Send string over serial connection
     * @param data String to send
     * @return Number of bytes sent, -1 on error
     */
    virtual int send(const std::string& data) = 0;
    
    /**
     * @brief Receive data from serial connection
     * @param buffer Buffer to store received data
     * @param maxLength Maximum number of bytes to receive
     * @return Number of bytes received, -1 on error
     */
    virtual int receive(uint8_t* buffer, size_t maxLength) = 0;
    
    /**
     * @brief Receive string from serial connection
     * @param data String to store received data
     * @param maxLength Maximum number of bytes to receive
     * @return Number of bytes received, -1 on error
     */
    virtual int receive(std::string& data, size_t maxLength) = 0;
    
    /**
     * @brief Check if data is available to read
     * @return Number of bytes available to read
     */
    virtual int available() const = 0;
    
    /**
     * @brief Flush input/output buffers
     */
    virtual void flush() = 0;
    
    /**
     * @brief Close the serial connection
     */
    virtual void close() = 0;
};

} // namespace jellED

#endif // __JELLED_I_SERIAL_H__
