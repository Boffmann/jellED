#include "serial/RaspiUart.h"
#include "utils/raspilogger.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstring>
#include <system_error>
#include <iostream>
#include <errno.h>
#include <sys/time.h>

namespace jellED {

RaspiUart::RaspiUart(const std::string portName, const int portNumber) 
    : fileDescriptor(-1), portName(portName), uartNumber(portNumber), initialized(false) {
    memset(&originalTios, 0, sizeof(originalTios));
    memset(&currentTios, 0, sizeof(currentTios));
}

RaspiUart::~RaspiUart() {
    if (initialized) {
        close();
    }
}

bool RaspiUart::initialize(const SerialConfig& config, const uint32_t baudRate) {
    if (initialized) {
        close();
    }
    
    this->config = config;
    this->baudRate = baudRate;
    
    // Validate port name
    if (portName.empty()) {
        std::cout << "Error: Port name is empty" << std::endl;
        return false;
    }
    
    // Open serial port (blocking mode for reliable communication)
    fileDescriptor = open(portName.c_str(), O_RDWR | O_NOCTTY);
    if (fileDescriptor < 0) {
        std::cout << "Error opening file descriptor for " << portName << ": " << strerror(errno) << std::endl;
        return false;
    }
    
    // Get current terminal settings
    if (tcgetattr(fileDescriptor, &originalTios) != 0) {
        std::cout << "Error getting current terminal settings: " << strerror(errno) << std::endl;
        ::close(fileDescriptor);
        fileDescriptor = -1;
        return false;
    }
    
    // Configure terminal settings
    if (!configureTermios()) {
        std::cout << "Error configuring terminal settings" << std::endl;
        ::close(fileDescriptor);
        fileDescriptor = -1;
        return false;
    }
    
    initialized = true;
    return true;
}

bool RaspiUart::isInitialized() const {
    return initialized && fileDescriptor >= 0;
}

int RaspiUart::send(const uint8_t* data, size_t length) {
    if (!isInitialized() || !data) {
        return -1;
    }
    
    if (length == 0) {
        return -1;
    }
    
    // Ensure all bytes are written by looping until complete
    size_t bytesWritten = 0;
    
    while (bytesWritten < length) {
        ssize_t written = write(fileDescriptor, data + bytesWritten, length - bytesWritten);
        
        if (written < 0) {
            // Error occurred
            return -1;
        }
        
        bytesWritten += written;
        
        // Small delay to allow system to process
        if (bytesWritten < length) {
            usleep(1000); // 1ms delay
        }
    }
    
    return static_cast<int>(bytesWritten);
}

int RaspiUart::send(const std::string& data) {
    if (!isInitialized()) {
        return -1;
    }
    
    if (data.empty()) {
        return -1;
    }
    
    // Ensure all bytes are written by looping until complete
    const char* buffer = data.c_str();
    size_t totalBytes = data.size();
    size_t bytesWritten = 0;
    
    while (bytesWritten < totalBytes) {
        ssize_t written = write(fileDescriptor, buffer + bytesWritten, totalBytes - bytesWritten);
        
        if (written < 0) {
            // Error occurred
            return -1;
        }
        
        bytesWritten += written;
        
        // Small delay to allow system to process
        if (bytesWritten < totalBytes) {
            usleep(1000); // 1ms delay
        }
    }
    
    return static_cast<int>(bytesWritten);
}

int RaspiUart::receive(uint8_t* buffer, size_t maxLength) {
    if (!isInitialized() || !buffer) {
        return -1;
    }
    
    if (maxLength == 0) {
        return -1;
    }
    
    // Read data from UART with timeout handling
    ssize_t totalRead = 0;
    ssize_t dataRead = 0;
    
    // Try to read all available data up to maxLength
    while (totalRead < static_cast<ssize_t>(maxLength)) {
        dataRead = read(fileDescriptor, buffer + totalRead, maxLength - totalRead);
        
        if (dataRead > 0) {
            totalRead += dataRead;
        } else if (dataRead == 0) {
            // No more data available (EOF or timeout)
            break;
        } else {
            // Error occurred
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No data available right now, but not an error
                break;
            } else {
                // Real error
                return -1;
            }
        }
    }
    
    return static_cast<int>(totalRead);
}

int RaspiUart::receive(std::string& buffer, size_t maxLength) {
    if (!isInitialized()) {
        return -1;
    }

    // Validate maxLength to prevent excessive memory allocation
    if (maxLength == 0 || maxLength > 8192) { // Reasonable upper limit
        return -1;
    }

    // Use stack allocation for small buffers, heap for large ones
    const size_t STACK_THRESHOLD = 256;
    char* charBuffer = nullptr;
    bool use_heap = maxLength > STACK_THRESHOLD;
    
    if (use_heap) {
        charBuffer = static_cast<char*>(malloc(maxLength));
        if (!charBuffer) {
            return -1; // Memory allocation failed
        }
    } else {
        charBuffer = static_cast<char*>(alloca(maxLength));
    }
    
    // Read data from UART with timeout handling
    ssize_t totalRead = 0;
    ssize_t dataRead = 0;
    
    // Try to read all available data up to maxLength
    while (totalRead < static_cast<ssize_t>(maxLength)) {
        dataRead = read(fileDescriptor, charBuffer + totalRead, maxLength - totalRead);
        
        if (dataRead > 0) {
            totalRead += dataRead;
        } else if (dataRead == 0) {
            // No more data available (EOF or timeout)
            break;
        } else {
            // Error occurred
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No data available right now, but not an error
                break;
            } else {
                // Real error
                if (use_heap) {
                    free(charBuffer);
                }
                return -1;
            }
        }
    }
    
    if (totalRead > 0) {
        // Only copy the actual bytes read, not the entire buffer
        buffer.assign(charBuffer, totalRead);
    } else {
        buffer.clear();
    }
    
    // Free heap memory if used
    if (use_heap) {
        free(charBuffer);
    }
    
    return static_cast<int>(totalRead);
}

int RaspiUart::available() const {
    if (!isInitialized()) {
        return 0;
    }
    
    int bytesAvailable = 0;
    if (ioctl(fileDescriptor, FIONREAD, &bytesAvailable) == 0) {
        return bytesAvailable;
    }
    
    return 0;
}

void RaspiUart::flush() {
    if (isInitialized()) {
        tcflush(fileDescriptor, TCIOFLUSH);
    }
}

void RaspiUart::close() {
    if (initialized) {
        if (fileDescriptor >= 0) {
            restoreTermios();
            ::close(fileDescriptor);
            fileDescriptor = -1;
        }
        initialized = false;
    }
}

bool RaspiUart::isOpen() const {
    return fileDescriptor >= 0;
}

speed_t RaspiUart::getLinuxBaudRate(uint32_t baudRate) const {
    std::cout << "Baud: " << baudRate << std::endl;
    switch (baudRate) {
        case 1200: return B1200;
        case 2400: return B2400;
        case 4800: return B4800;
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        case 230400: return B230400;
        default: return B115200;
    }
}

bool RaspiUart::configureTermios() {
    // Copy original settings
    currentTios = originalTios;
    
    // Set baud rate
    speed_t baudRate = getLinuxBaudRate(this->baudRate);
    if (cfsetispeed(&currentTios, baudRate) != 0 || 
        cfsetospeed(&currentTios, baudRate) != 0) {
        std::cout << "Error setting baud rate: " << strerror(errno) << std::endl;
        return false;
    }
    
    // Configure data bits, stop bits, and parity
    currentTios.c_cflag &= ~CSIZE; // Clear data size bits
    switch (config.dataBits) {
        case 5: currentTios.c_cflag |= CS5; break;
        case 6: currentTios.c_cflag |= CS6; break;
        case 7: currentTios.c_cflag |= CS7; break;
        case 8: currentTios.c_cflag |= CS8; break;
        default: currentTios.c_cflag |= CS8; break;
    }
    
    // Configure stop bits
    if (config.stopBits == 2) {
        currentTios.c_cflag |= CSTOPB;
    } else {
        currentTios.c_cflag &= ~CSTOPB;
    }
    
    // Configure parity
    if (config.parity) {
        currentTios.c_cflag |= PARENB;
        currentTios.c_cflag &= ~PARODD; // Even parity
    } else {
        currentTios.c_cflag &= ~PARENB;
    }
    
    // Configure flow control
    if (config.flowControl) {
        currentTios.c_cflag |= CRTSCTS;
    } else {
        currentTios.c_cflag &= ~CRTSCTS;
    }
    
    // Set raw mode
    currentTios.c_cflag |= CLOCAL | CREAD;
    currentTios.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    currentTios.c_oflag &= ~OPOST;
    currentTios.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL);
    
    // Set timeout (convert ms to deciseconds, minimum 1)
    uint8_t timeout_deciseconds = (config.timeoutMs + 99) / 100; // Round up division
    if (timeout_deciseconds < 1) timeout_deciseconds = 1;
    currentTios.c_cc[VTIME] = timeout_deciseconds;
    currentTios.c_cc[VMIN] = 0; // Non-blocking
    
    // Apply settings
    if (tcsetattr(fileDescriptor, TCSANOW, &currentTios) != 0) {
        std::cout << "Error applying terminal settings: " << strerror(errno) << std::endl;
        return false;
    }
    
    return true;
}

void RaspiUart::restoreTermios() {
    if (fileDescriptor >= 0) {
        tcsetattr(fileDescriptor, TCSANOW, &originalTios);
    }
}

} // namespace jellED 
