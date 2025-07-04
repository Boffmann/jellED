#ifndef __JELLED_RASPI_LOGGER_H__
#define __JELLED_RASPI_LOGGER_H__

#include "ILogger.h"

class RaspiLogger : public ILogger {
public:
    void log(const std::string message);
};

#endif
