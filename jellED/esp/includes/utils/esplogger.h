#ifndef __JELLED_ESP_LOGGER_H__
#define __JELLED_ESP_LOGGER_H__

#include "ILogger.h"

namespace jellED {

class EspLogger : public ILogger {
public:
    void log(const std::string message);
};

} // namespace jellED

#endif
