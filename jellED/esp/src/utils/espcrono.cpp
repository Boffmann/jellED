#include "espcrono.h"
#include <Arduino.h>

virtual unsigned long EspCrono::currentTimeMicros() {
    return micros();
}
