#ifndef _PATTERN_COLORED_AMPLITUDE_H_
#define _PATTERN_COLORED_AMPLITUDE_H_

#include "patternBlueprint.h"

namespace jellED {

class RainbowPattern : public PatternBlueprint {
private:
    const unsigned long time_per_color_micros;
    const unsigned long brightness_decay_micros;

public:
    RainbowPattern(unsigned long startTime, unsigned long pattern_duration_micros);
    RainbowPattern(unsigned long startTime, unsigned long pattern_duration_micros,
                   unsigned long brightness_decay_micros);
    ~RainbowPattern() = default;

    void update_pattern(const AudioFeatures& features, unsigned long current_time_micros,
                        pattern_color* output, int num_leds) override;
};

} // namespace jellED

#endif
