#ifndef __JELLED_TEST_PLATFORMUTILS_H__
#define __JELLED_TEST_PLATFORMUTILS_H__

#include "../../../IPlatformUtils.h"
#include "../../../ICrono.h"
#include "../../../ILogger.h"

#include <memory>

namespace jellED {

class TestPlatformUtils : public IPlatformUtils {
private:
    std::unique_ptr<ICrono> _crono;
    std::unique_ptr<ILogger> _logger;
public:
    TestPlatformUtils();
    ILogger& logger();
    ICrono& crono();
};

} // namespace jellED

#endif
