#ifndef _PATTERN_COLORED_AMPLITUDE_H_
#define _PATTERN_COLORED_AMPLITUDE_H_

#include "patternBlueprint.h"

namespace jellED {

class RainbowPattern : public PatternBlueprint {
private:
    const unsigned long time_per_color_micros;
    const unsigned long brightness_decay_micros;
    uint8_t current_color_index;
    uint8_t current_brightness;

protected:
    void init();

public:
    RainbowPattern(unsigned long startTime, int num_leds, unsigned long pattern_duration_micros);
    RainbowPattern(unsigned long startTime, int num_leds, unsigned long pattern_duration_micros, unsigned long brightness_decay_micros);
    ~RainbowPattern() {};
    void update_pattern(bool is_beat, unsigned long current_time_micros);
};

} // namespace jellED

#endif
