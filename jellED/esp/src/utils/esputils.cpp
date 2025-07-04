#include "esputils.h"

namespace jellED {

EspPlatformUtils::EspPlatformUtils() {
   this->_logger = EspLogger();
   this->_crono = EspCrono(); 
}

ILogger& EspPlatformUtils::logger() {
    return this->_logger;
}

ICrono& EspPlatformUtils::crono() {
    return this->_crono;
}

} // namespace jellED
