#include "pulseFlashPattern.h"
#include "pattern_colors.h"

namespace jellED {

PulseFlashPattern::PulseFlashPattern(unsigned long startTime,
                                     unsigned long pattern_duration_micros)
    : PatternBlueprint(startTime, PatternType::PULSE_FLASH, pattern_duration_micros),
      flash_r(0.0f), flash_g(0.0f), flash_b(0.0f),
      flash_level(0.0f),
      ambient_brightness(0.0f) {}

void PulseFlashPattern::update_pattern(const AudioFeatures& features,
                                       unsigned long current_time_micros,
                                       pattern_color* output, int num_leds) {
    // ── Ambient: smoothed average of all bands ─────────────────────────────────
    float overall = (static_cast<float>(features.volumeLow)
                   + static_cast<float>(features.volumeMid)
                   + static_cast<float>(features.volumeHigh)) / 3.0f;
    ambient_brightness = AMBIENT_SMOOTHING * overall
                       + (1.0f - AMBIENT_SMOOTHING) * ambient_brightness;

    // ── Beat flash ────────────────────────────────────────────────────────────
    if (features.isBeat() && should_react_to_beat) {
        time_of_last_beat = current_time_micros;
        flash_level = 255.0f;

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

    // ── Decay flash linearly over FLASH_DECAY_MICROS ──────────────────────────
    if (flash_level > 0.0f) {
        unsigned long elapsed = current_time_micros - time_of_last_beat;
        float fraction = 1.0f - static_cast<float>(elapsed) / FLASH_DECAY_MICROS;
        flash_level = (fraction > 0.0f) ? 255.0f * fraction : 0.0f;
    }

    // ── Composite: flash on top of dim ambient ─────────────────────────────────
    float ambient_scale = ambient_brightness / 255.0f * 0.3f; // ambient at 30% max
    uint8_t br;
    uint8_t r, g, b;
    if (flash_level > 0.0f) {
        float fl = flash_level / 255.0f;
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
