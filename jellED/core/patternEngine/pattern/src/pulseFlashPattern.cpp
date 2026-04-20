#include "pulseFlashPattern.h"
#include "pattern_colors.h"

namespace jellED {

PulseFlashPattern::PulseFlashPattern(unsigned long startTime,
                                     unsigned long pattern_duration_micros)
    : PatternBlueprint(startTime, PatternType::PULSE_FLASH, pattern_duration_micros),
      flash_filter_(0.0f, FLASH_TAU_RISE_S, FLASH_TAU_DECAY_S),
      ambient_filter_(0.0f, AMBIENT_TAU_RISE_S, AMBIENT_TAU_DECAY_S),
      last_update_micros_(0),
      flash_r(0.0f), flash_g(0.0f), flash_b(0.0f) {}

void PulseFlashPattern::update_pattern(const AudioFeatures& features,
                                       unsigned long current_time_micros,
                                       pattern_color* output, int num_leds) {
    // ── dt bookkeeping ────────────────────────────────────────────────────────
    float dt_seconds;
    if (last_update_micros_ == 0) {
        dt_seconds = 0.0f;
    } else {
        dt_seconds = static_cast<float>(current_time_micros - last_update_micros_) * 1e-6f;
    }
    last_update_micros_ = current_time_micros;

    // ── Ambient: slow asymmetric smoother over all-bands average ─────────────
    const float overall = (static_cast<float>(features.volumeLow)
                         + static_cast<float>(features.volumeMid)
                         + static_cast<float>(features.volumeHigh)) / 3.0f;
    const float ambient_brightness = ambient_filter_.update(overall, dt_seconds);

    // ── Beat flash: instant snap on beat, exponential decay afterwards ────────
    if (features.isBeat() && should_react_to_beat) {
        time_of_last_beat = current_time_micros;
        flash_filter_.reset(255.0f);

        uint8_t flags = features.beatFlags;
        int bands = ((flags & AudioFeatures::BEAT_LOW)  ? 1 : 0)
                  + ((flags & AudioFeatures::BEAT_MID)  ? 1 : 0)
                  + ((flags & AudioFeatures::BEAT_HIGH) ? 1 : 0);

        if (bands > 1) {
            flash_r = 255.0f; flash_g = 255.0f; flash_b = 255.0f; // multi-band → white
        } else if (flags & AudioFeatures::BEAT_LOW) {
            flash_r = 255.0f; flash_g = 140.0f; flash_b = 0.0f;   // warm orange
        } else if (flags & AudioFeatures::BEAT_MID) {
            flash_r = 255.0f; flash_g = 220.0f; flash_b = 180.0f; // warm white
        } else {
            flash_r = 100.0f; flash_g = 160.0f; flash_b = 255.0f; // cool blue
        }
    }

    // Decay the flash toward 0. Because input < current, τ_decay applies.
    const float flash_level = flash_filter_.update(0.0f, dt_seconds);

    // ── Composite: flash on top of dim ambient ────────────────────────────────
    const float ambient_scale = ambient_brightness / 255.0f * 0.3f; // ambient at 30% max
    uint8_t br;
    uint8_t r, g, b;
    if (flash_level > 1.0f) {  // below ~0.4% brightness, not worth computing
        const float fl = flash_level / 255.0f;
        r = static_cast<uint8_t>(flash_r * fl);
        g = static_cast<uint8_t>(flash_g * fl);
        b = static_cast<uint8_t>(flash_b * fl);
        br = 255;
    } else {
        // Ambient: neutral warm white scaled by volume
        r = static_cast<uint8_t>(255.0f * ambient_scale);
        g = static_cast<uint8_t>(200.0f * ambient_scale);
        b = static_cast<uint8_t>(150.0f * ambient_scale);
        br = 255;
    }

    pattern_color c{r, g, b, br};
    for (int i = 0; i < num_leds; ++i) {
        output[i] = c;
    }
}

} // namespace jellED
