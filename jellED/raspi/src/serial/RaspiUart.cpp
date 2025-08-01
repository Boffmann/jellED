#include "serial/RaspiUart.h"
#include "utils/raspilogger.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstring>
#include <system_error>

namespace jellED {

RaspiUart::RaspiUart() 
    : fileDescriptor(-1), uartNumber(0), initialized(false) {
    memset(&originalTios, 0, sizeof(originalTios));
    memset(&currentTios, 0, sizeof(currentTios));
}

RaspiUart::~RaspiUart() {
    if (initialized) {
        close();
    }
}

bool RaspiUart::initialize(const UartConfig& uartConfig) {
    if (initialized) {
        close();
    }
    
    config = uartConfig;
    portName = config.portName;
    uartNumber = config.uartNumber;
    
    // Open serial port
    fileDescriptor = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fileDescriptor < 0) {
        return false;
    }
    
    // Get current terminal settings
    if (tcgetattr(fileDescriptor, &originalTios) != 0) {
        ::close(fileDescriptor);
        fileDescriptor = -1;
        return false;
    }
    
    // Configure terminal settings
    if (!configureTermios()) {
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
    if (!isInitialized()) {
        return -1;
    }
    
    ssize_t written = write(fileDescriptor, data, length);
    return static_cast<int>(written);
}

int RaspiUart::receive(uint8_t* buffer, size_t maxLength) {
    if (!isInitialized()) {
        return -1;
    }
    
    ssize_t dataRead = read(fileDescriptor, buffer, maxLength);

    return static_cast<int>(dataRead);
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

UartConfig RaspiUart::getConfig() const {
    return config;
}

bool RaspiUart::setPins(int txPin, int rxPin) {
    // On RaspberryPi, pins are typically configured through device tree or GPIO
    // This is a placeholder for pin configuration
    return true;
}

std::string RaspiUart::getPortName() const {
    return portName;
}

int RaspiUart::getUartNumber() const {
    return uartNumber;
}

bool RaspiUart::isLoopbackEnabled() const {
    return config.enableLoopback;
}

void RaspiUart::setPortName(const std::string& port) {
    if (!initialized) {
        portName = port;
        config.portName = port;
    }
}

int RaspiUart::getFileDescriptor() const {
    return fileDescriptor;
}

bool RaspiUart::isOpen() const {
    return fileDescriptor >= 0;
}

speed_t RaspiUart::getLinuxBaudRate(uint32_t baudRate) const {
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
    speed_t baudRate = getLinuxBaudRate(config.baudRate);
    if (cfsetispeed(&currentTios, baudRate) != 0 || 
        cfsetospeed(&currentTios, baudRate) != 0) {
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
    
    // Set timeout
    currentTios.c_cc[VTIME] = config.timeoutMs / 100; // Deciseconds
    currentTios.c_cc[VMIN] = 0; // Non-blocking
    
    // Apply settings
    if (tcsetattr(fileDescriptor, TCSANOW, &currentTios) != 0) {
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