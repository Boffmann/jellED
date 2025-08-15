#ifndef _PATTERN_COLORED_AMPLITUDE_H_
#define _PATTERN_COLORED_AMPLITUDE_H_

#include "patternBlueprint.h"

namespace jellED {

class ColoredAmplitude : public PatternBlueprint {
public:
    ColoredAmplitude(IPlatformUtils& pUtils, int num_leds);
    ~ColoredAmplitude() {};
    void update_pattern(long time_since_beat_micros);

protected:
    void init();
};

} // namespace jellED

#endif
