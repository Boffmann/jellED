#ifndef _PULSE_FLASH_PATTERN_H_
#define _PULSE_FLASH_PATTERN_H_

#include "patternBlueprint.h"

namespace jellED {

// Sharp flash on beat that decays quickly (250 ms), with a dim ambient glow
// tracking overall volume. Beat color is band-specific:
//   BEAT_LOW  → warm orange  {255, 140,   0}
//   BEAT_MID  → warm white   {255, 220, 180}
//   BEAT_HIGH → cool blue    {100, 160, 255}
//   multiple  → white        {255, 255, 255}
class PulseFlashPattern : public PatternBlueprint {
private:
    float flash_r, flash_g, flash_b; // current flash color (fades to zero)
    float flash_level;               // current flash brightness [0..255]
    float ambient_brightness;        // smoothed overall volume

    static constexpr float FLASH_DECAY_MICROS = 250000.0f; // 250 ms full decay
    static constexpr float AMBIENT_SMOOTHING  = 0.10f;

public:
    PulseFlashPattern(unsigned long startTime,
                      unsigned long pattern_duration_micros = 10000000UL);

    void update_pattern(const AudioFeatures& features, unsigned long current_time_micros,
                        pattern_color* output, int num_leds) override;
};

} // namespace jellED

#endif
