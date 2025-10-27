#ifndef __JELLED_TEST_PLATFORMUTILS_H__
#define __JELLED_TEST_PLATFORMUTILS_H__

#include "pUtils//IPlatformUtils.h"
#include "pUtils/ICrono.h"
#include "pUtils/ILogger.h"

#include <memory>

namespace jellED {

class TestPlatformUtils : public IPlatformUtils {
private:
    std::unique_ptr<ICrono> _crono;
    std::unique_ptr<ILogger> _logger;
    std::unique_ptr<IInputOutput> _io;
public:
    TestPlatformUtils();
    ILogger& logger();
    ICrono& crono();
    IInputOutput& io();
};

} // namespace jellED

#endif
