#ifndef __JELLED_I_LOGGER_H__
#define __JELLED_I_LOGGER_H__

#include <string>

namespace jellED {

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(const std::string message) = 0;
};

} // namespace jellED

#endif
