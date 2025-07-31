#include "espcrono.h"
#include <Arduino.h>

namespace jellED {

unsigned long EspCrono::currentTimeMicros() {
    return micros();
}

} // namespace jellED
