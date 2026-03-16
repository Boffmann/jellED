#include "breathingGlowPattern.h"
#include "pattern_colors.h"

namespace jellED {

BreathingGlowPattern::BreathingGlowPattern(unsigned long startTime,
                                           unsigned long pattern_duration_micros)
    : PatternBlueprint(startTime, PatternType::BREATHING_GLOW, pattern_duration_micros),
      smoothed_brightness(0.0f),
      beat_brightness(0.0f) {}

void BreathingGlowPattern::update_pattern(const AudioFeatures& features,
                                          unsigned long current_time_micros,
                                          pattern_color* output, int num_leds) {
    // ── Smoothed brightness from bass volume ──────────────────────────────────
    float target = static_cast<float>(features.volumeLow);
    smoothed_brightness = SMOOTHING_ALPHA * target + (1.0f - SMOOTHING_ALPHA) * smoothed_brightness;

    // ── Beat spike ────────────────────────────────────────────────────────────
    if (features.isBeat() && should_react_to_beat) {
        beat_brightness = 255.0f;
        time_of_last_beat = current_time_micros;
    }

    if (beat_brightness > 0.0f) {
        unsigned long elapsed_ms = (current_time_micros - time_of_last_beat) / 1000UL;
        float decayed = 255.0f - BEAT_DECAY_RATE * static_cast<float>(elapsed_ms);
        beat_brightness = (decayed > 0.0f) ? decayed : 0.0f;
    }

    float brightness = (beat_brightness > smoothed_brightness) ? beat_brightness : smoothed_brightness;
    if (brightness > 255.0f) brightness = 255.0f;

    // ── Color from spectral tilt: 30° (orange) → 200° (cyan-blue) ────────────
    // spectralTilt: 0=treble-heavy, 255=bass-heavy
    float tilt_norm = static_cast<float>(features.spectralTilt) / 255.0f;
    float hue = 30.0f + tilt_norm * 170.0f; // orange..cyan-blue
    pattern_color c = hsvToRgb(hue, 1.0f, 1.0f);
    c.brightness = static_cast<uint8_t>(brightness);

    for (int i = 0; i < num_leds; ++i) {
        output[i] = c;
    }
}

} // namespace jellED
