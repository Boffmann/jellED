#include "esplogger.h"
#include <Arduino.h>

void EspLogger::log(const std::string message) {
    Serial.println(message.c_str());
}
