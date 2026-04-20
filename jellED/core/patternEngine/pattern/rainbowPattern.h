#ifndef _PATTERN_COLORED_AMPLITUDE_H_
#define _PATTERN_COLORED_AMPLITUDE_H_

#include "patternBlueprint.h"

namespace jellED {

// Note on flash decay: RainbowPattern uses a stateless exp() computation
// based on time_of_last_beat rather than an ExpFilter instance. The flash is
// an event-triggered decay from a known peak — the closed-form exponential
// is simpler than ExpFilter's stateful recurrence AND correctly follows
// external resets of time_of_last_beat (e.g. via shouldReactToBeat).
// See audio_reactive_led_strip_learnings.md §4.1 for the design rationale.
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
