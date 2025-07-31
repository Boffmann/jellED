#include "../uart.h"
#include <cstring>
#include <algorithm>

namespace jellED {

// Base UART implementation with common functionality
class UartBase : public IUart {
protected:
    UartConfig config;
    bool initialized;
    int txPin;
    int rxPin;
    
public:
    UartBase() : initialized(false), txPin(-1), rxPin(-1) {}
    
    virtual ~UartBase() {
        if (initialized) {
            close();
        }
    }
    
    bool initialize(const SerialConfig& config) override {
        // This should be overridden by platform-specific implementations
        return false;
    }
    
    bool initialize(const UartConfig& uartConfig) override {
        config = uartConfig;
        initialized = true;
        return true;
    }
    
    bool isAvailable() const override {
        return initialized;
    }
    
    int send(const std::string& data) override {
        return send(reinterpret_cast<const uint8_t*>(data.c_str()), data.length());
    }
    
    int receive(std::string& data, size_t maxLength) override {
        std::vector<uint8_t> buffer(maxLength);
        int bytesRead = receive(buffer.data(), maxLength);
        if (bytesRead > 0) {
            data.assign(reinterpret_cast<const char*>(buffer.data()), bytesRead);
        } else {
            data.clear();
        }
        return bytesRead;
    }
    
    UartConfig getConfig() const override {
        return config;
    }
    
    std::string getPortName() const override {
        return config.portName;
    }
    
    int getUartNumber() const override {
        return config.uartNumber;
    }
    
    bool isLoopbackEnabled() const override {
        return config.enableLoopback;
    }
    
    bool setPins(int tx, int rx) override {
        txPin = tx;
        rxPin = rx;
        return true;
    }
    
    void flush() override {
        // Platform-specific implementations should override this
    }
    
    void close() override {
        initialized = false;
    }
};

} // namespace jellED 