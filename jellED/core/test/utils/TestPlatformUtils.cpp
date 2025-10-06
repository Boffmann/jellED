#include "TestPlatformUtils.h"

#include <iostream>

namespace jellED {

class TestCrono : public ICrono {
private:
    unsigned long current_time_micros = 0;
public:
    unsigned long currentTimeMicros() {
        current_time_micros += 1000000;
        return current_time_micros;
    }
};

class TestLogger : public ILogger {
public:
    void log(const std::string message) {
        std::cout << message << std::endl;
    }
};

class TestInputOutput : public IInputOutput {
public:
    BinaryState digitalReadPin(uint8_t pin) {
        return STATE_HIGH;
    } 
};

TestPlatformUtils::TestPlatformUtils() {
    this->_crono = std::make_unique<TestCrono>();
    this->_logger = std::make_unique<TestLogger>();
    this->_io = std::make_unique<TestInputOutput>();
}

ILogger& TestPlatformUtils::logger() {
    return *this->_logger.get();
}

ICrono& TestPlatformUtils::crono() {
    return *this->_crono.get();
}

IInputOutput& TestPlatformUtils::io() {
    return *this->_io.get();
}

} // namespace jellED
