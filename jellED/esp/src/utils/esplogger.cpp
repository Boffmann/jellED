#include "esplogger.h"
#include <Arduino.h>

namespace jellED {

void EspLogger::log(const std::string message) {
    Serial.println(message.c_str());
}

} // namespace jellED
