#include "utils/raspilogger.h"
#include <iostream>

namespace jellED {

void RaspiLogger::log(const std::string message) {
    std::cout << message << std::endl;
}

} // namespace jellED
