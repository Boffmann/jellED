#ifndef __JELLED_I_PLATTFORMUTILS_H__
#define __JELLED_I_PLATTFORMUTILS_H__

#include "ILogger.h"
#include "ICrono.h"

class IPlatformUtils {
public:
    virtual ~IPlatformUtils() = default;
    virtual ILogger& logger() = 0;
    virtual ICrono& crono() = 0;
};
#endif
