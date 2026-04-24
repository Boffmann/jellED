#include "GuiPlatformUtils.h"

#include <iostream>

namespace jellED {

void GuiLogger::log(const std::string message) {
    std::cout << message << std::endl;
}

GuiCrono::GuiCrono() : start_(std::chrono::steady_clock::now()) {}

unsigned long GuiCrono::currentTimeMicros() {
    const auto now = std::chrono::steady_clock::now();
    return static_cast<unsigned long>(
        std::chrono::duration_cast<std::chrono::microseconds>(now - start_).count());
}

BinaryState GuiInputOutput::digitalReadPin(uint8_t /*pin*/) {
    return STATE_LOW;
}

} // namespace jellED
