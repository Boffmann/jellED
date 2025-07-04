# jellED Serial Library

A cross-platform serial communication library designed for UART communication between RaspberryPi and ESP32 devices.

## Overview

The jellED Serial Library provides a unified interface for serial communication across different platforms. It currently supports:

- **UART Communication**: Full UART support with configurable parameters
- **ESP32 Platform**: Native ESP32 UART driver integration
- **RaspberryPi Platform**: Linux serial port support using termios

## Architecture

The library follows a layered architecture:

```
ISerial (Core Interface)
    ↓
IUart (UART-specific Interface)
    ↓
Platform Implementations (EspUart, RaspiUart)
```

### Core Interfaces

#### ISerial
The base serial communication interface providing:
- `initialize()` - Initialize serial connection
- `send()` - Send data over serial
- `receive()` - Receive data from serial
- `available()` - Check for available data
- `flush()` - Flush buffers
- `close()` - Close connection

#### IUart
UART-specific interface extending ISerial with:
- `setPins()` - Configure TX/RX pins
- `getPortName()` - Get UART port name
- `getUartNumber()` - Get UART number
- Platform-specific configuration options

## Configuration

### SerialConfig
Basic serial configuration:
```cpp
SerialConfig config;
config.baudRate = 115200;
config.dataBits = 8;
config.stopBits = 1;
config.parity = false;
config.flowControl = false;
config.timeoutMs = 1000;
```

### UartConfig
UART-specific configuration extending SerialConfig:
```cpp
UartConfig config;
config.portName = "/dev/ttyAMA0";  // RaspberryPi
config.portName = "UART0";         // ESP32
config.uartNumber = 0;
config.invertTx = false;
config.invertRx = false;
config.enableLoopback = false;
```

## Platform Support

### ESP32

**Header**: `jellED/esp/includes/serial/EspUart.h`
**Implementation**: `jellED/esp/src/serial/EspUart.cpp`

Features:
- Native ESP32 UART driver integration
- Support for UART0, UART1, UART2
- Configurable GPIO pins
- Hardware flow control support
- Loopback mode for testing

Example:
```cpp
#include "EspUart.h"

jellED::serial::EspUart uart;
UartConfig config;
config.portName = "UART0";
config.baudRate = 115200;
config.uartNumber = 0;

if (uart.initialize(config)) {
    uart.setPins(17, 16);  // TX: GPIO17, RX: GPIO16
    uart.send("Hello from ESP32!");
}
```

### RaspberryPi

**Header**: `jellED/raspi/include/serial/RaspiUart.h`
**Implementation**: `jellED/raspi/src/serial/RaspiUart.cpp`

Features:
- Linux serial port support
- Termios configuration
- Support for standard UART ports
- Configurable timeout and buffer settings

Example:
```cpp
#include "RaspiUart.h"

jellED::serial::RaspiUart uart;
UartConfig config;
config.portName = "/dev/ttyAMA0";
config.baudRate = 115200;

if (uart.initialize(config)) {
    uart.send("Hello from RaspberryPi!");
}
```

## Usage Examples

### Basic Communication

```cpp
#include "uart.h"

// Create platform-specific UART instance
jellED::serial::EspUart uart;  // or RaspiUart

// Configure and initialize
UartConfig config;
config.portName = "UART0";
config.baudRate = 115200;
config.dataBits = 8;
config.stopBits = 1;

if (uart.initialize(config)) {
    // Send data
    std::string message = "Hello World!";
    int sent = uart.send(message);
    
    // Receive data
    std::string response;
    int received = uart.receive(response, 256);
    
    // Check available data
    int available = uart.available();
    
    // Clean up
    uart.close();
}
```

### Continuous Communication Loop

```cpp
#include "uart.h"
#include <thread>
#include <chrono>

jellED::serial::EspUart uart;
UartConfig config;
config.portName = "UART0";
config.baudRate = 115200;

if (uart.initialize(config)) {
    while (true) {
        // Check for incoming data
        if (uart.available() > 0) {
            std::string message;
            int received = uart.receive(message, 256);
            if (received > 0) {
                // Process received message
                std::cout << "Received: " << message << std::endl;
                
                // Send response
                uart.send("Acknowledged: " + message);
            }
        }
        
        // Small delay to prevent busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
```

## Building

### CMake Integration

The serial library is automatically included in the core CMake build:

```cmake
# Core CMakeLists.txt already includes serial library
add_subdirectory(${CORE_LIBS_DIR}/serial ${CMAKE_BINARY_DIR}/core/serial)
```

### Platform-Specific Dependencies

#### ESP32
- ESP-IDF UART driver
- FreeRTOS (for timeouts)

#### RaspberryPi
- Linux system headers
- Termios library

## Testing

### Loopback Testing

Both platforms support loopback mode for testing:

```cpp
UartConfig config;
config.enableLoopback = true;
config.portName = "UART0";

if (uart.initialize(config)) {
    uart.send("Test message");
    std::string response;
    uart.receive(response, 256);
    // response should contain "Test message"
}
```

### Cross-Platform Communication

For communication between RaspberryPi and ESP32:

1. **Physical Connection**:
   - RaspberryPi TX (GPIO14) → ESP32 RX (GPIO16)
   - RaspberryPi RX (GPIO15) → ESP32 TX (GPIO17)
   - Connect GND between devices

2. **Configuration**:
   - Use same baud rate (e.g., 115200)
   - Same data format (8N1)
   - Disable flow control

3. **Code Example**:
   ```cpp
   // RaspberryPi side
   RaspiUart uart;
   UartConfig config;
   config.portName = "/dev/ttyAMA0";
   config.baudRate = 115200;
   uart.initialize(config);
   
   // ESP32 side
   EspUart uart;
   UartConfig config;
   config.portName = "UART0";
   config.baudRate = 115200;
   uart.initialize(config);
   uart.setPins(17, 16);  // TX: 17, RX: 16
   ```

## Error Handling

The library provides error handling through return values:

- `initialize()`: Returns `true` on success, `false` on failure
- `send()`: Returns number of bytes sent, `-1` on error
- `receive()`: Returns number of bytes received, `-1` on error
- `available()`: Returns number of bytes available, `0` if none

## Thread Safety

The current implementation is not thread-safe. For multi-threaded applications:

1. Use separate UART instances per thread
2. Implement external synchronization
3. Consider using mutex protection for shared UART instances

## Performance Considerations

- **Buffer Sizes**: ESP32 uses 1024-byte buffers by default
- **Timeout Settings**: Adjust based on application requirements
- **Baud Rate**: Higher baud rates for faster communication
- **Flow Control**: Enable for reliable high-speed communication

## Future Enhancements

- [ ] SPI support
- [ ] I2C support
- [ ] USB Serial support
- [ ] Thread-safe implementations
- [ ] Async/await support
- [ ] Protocol implementations (Modbus, etc.)

## Contributing

When adding new serial protocols or platform support:

1. Implement the appropriate interface (`ISerial` or `IUart`)
2. Follow the existing naming conventions
3. Add platform-specific CMake configuration
4. Include comprehensive error handling
5. Add examples and documentation 