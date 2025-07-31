#ifndef __JELLED_RASPI_LOGGER_H__
#define __JELLED_RASPI_LOGGER_H__

#include "ILogger.h"

namespace jellED {

class RaspiLogger : public ILogger {
public:
    void log(const std::string message);
};

} // namespace jellED

#endif
