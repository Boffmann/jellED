#include "esputils.h"

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
