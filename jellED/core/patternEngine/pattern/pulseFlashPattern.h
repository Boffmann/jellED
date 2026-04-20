#ifndef _PULSE_FLASH_PATTERN_H_
#define _PULSE_FLASH_PATTERN_H_

#include "expFilter.h"
#include "patternBlueprint.h"

namespace jellED {

// Sharp flash on beat that decays quickly (~250 ms), with a dim ambient glow
// tracking overall volume. Beat color is band-specific:
//   BEAT_LOW  → warm orange  {255, 140,   0}
//   BEAT_MID  → warm white   {255, 220, 180}
//   BEAT_HIGH → cool blue    {100, 160, 255}
//   multiple  → white        {255, 255, 255}
class PulseFlashPattern : public PatternBlueprint {
private:
    // Flash: instant snap (via reset) on beat, exponential decay afterwards.
    // tau_rise is nominal only — we use reset() to spike on beat.
    ExpFilter<float> flash_filter_;
    // Ambient: slow asymmetric smoother tracking overall volume — responds to
    // song sections, not individual beats.
    ExpFilter<float> ambient_filter_;
    unsigned long last_update_micros_;

    float flash_r, flash_g, flash_b; // current flash color (fades to zero)

    // Time constants (seconds).
    // Exponential decays to ~5% in 3×tau, so FLASH_TAU_DECAY=80ms ≈ old 250ms linear-to-zero.
    static constexpr float FLASH_TAU_RISE_S    = 0.001f;
    static constexpr float FLASH_TAU_DECAY_S   = 0.080f;
    static constexpr float AMBIENT_TAU_RISE_S  = 0.300f; // tracks song sections
    static constexpr float AMBIENT_TAU_DECAY_S = 0.800f; // lingers through quiet

public:
    PulseFlashPattern(unsigned long startTime,
                      unsigned long pattern_duration_micros = 10000000UL);

    void update_pattern(const AudioFeatures& features, unsigned long current_time_micros,
                        pattern_color* output, int num_leds) override;
};

} // namespace jellED

#endif
