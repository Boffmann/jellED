#ifndef __JELLED_GUI_PLATFORM_UTILS_H__
#define __JELLED_GUI_PLATFORM_UTILS_H__

#include "pUtils/IPlatformUtils.h"

#include <chrono>
#include <string>

namespace jellED {

class GuiLogger : public ILogger {
public:
    void log(const std::string message) override;
};

class GuiCrono : public ICrono {
public:
    GuiCrono();
    unsigned long currentTimeMicros() override;

private:
    std::chrono::steady_clock::time_point start_;
};

// Stub implementation — the Qt GUI has no GPIO. PatternEngine never calls into
// io(), but IPlatformUtils requires the method so we provide a no-op.
class GuiInputOutput : public IInputOutput {
public:
    BinaryState digitalReadPin(uint8_t pin) override;
};

class GuiPlatformUtils : public IPlatformUtils {
public:
    ILogger& logger() override { return logger_; }
    ICrono& crono() override { return crono_; }
    IInputOutput& io() override { return io_; }

private:
    GuiLogger logger_;
    GuiCrono crono_;
    GuiInputOutput io_;
};

} // namespace jellED

#endif
