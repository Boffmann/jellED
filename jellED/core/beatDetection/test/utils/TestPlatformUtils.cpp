#include "TestPlatformUtils.h"

#include <iostream>

namespace jellED {

class TestCrono : public ICrono {
public:
    unsigned long currentTimeMicros() {
        return 0L;
    }
};

class TestLogger : public ILogger {
public:
    void log(const std::string message) {
        std::cout << message << std::endl;
    }
};

TestPlatformUtils::TestPlatformUtils() {
    this->_crono = std::make_unique<TestCrono>();
    this->_logger = std::make_unique<TestLogger>();
}

ILogger& TestPlatformUtils::logger() {
    return *this->_logger.get();
}

ICrono& TestPlatformUtils::crono() {
    return *this->_crono.get();
}

} // namespace jellED