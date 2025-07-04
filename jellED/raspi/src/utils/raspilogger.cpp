#include "utils/raspilogger.h"
#include <iostream>

void RaspiLogger::log(const std::string message) {
    std::cout << message << std::endl;
}
