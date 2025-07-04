#include "utils/raspiutils.h"

namespace jellED {

RaspiPlatformUtils::RaspiPlatformUtils() {
    this->_logger = RaspiLogger();
    this->_crono = RaspiCrono();
}

ILogger& RaspiPlatformUtils::logger() {
    return this->_logger;
}

ICrono& RaspiPlatformUtils::crono() {
    return this->_crono;
}

} // namespace jellED
