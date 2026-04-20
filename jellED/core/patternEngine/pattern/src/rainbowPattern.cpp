#include "rainbowPattern.h"

#include <cmath>

#include "pattern_colors.h"

namespace jellED {

static constexpr pattern_color rainbow_colors[] = {RED, YELLOW, GREEN, CYAN, BLUE, MAGENTA};
static constexpr int NUM_RAINBOW_COLORS = sizeof(rainbow_colors) / sizeof(rainbow_colors[0]);
static constexpr unsigned long DEFAULT_BRIGHTNESS_DECAY_MICROS = 1000000;

RainbowPattern::RainbowPattern(unsigned long startTime, unsigned long pattern_duration_micros)
    : RainbowPattern(startTime, pattern_duration_micros, DEFAULT_BRIGHTNESS_DECAY_MICROS) {
}

RainbowPattern::RainbowPattern(unsigned long startTime, unsigned long pattern_duration_micros,
                               unsigned long brightness_decay_micros)
    : PatternBlueprint(startTime, PatternType::RAINBOW, pattern_duration_micros),
      time_per_color_micros{pattern_duration_micros / NUM_RAINBOW_COLORS},
      brightness_decay_micros{brightness_decay_micros} {
}

void RainbowPattern::update_pattern(const AudioFeatures& features, unsigned long current_time_micros,
                                    pattern_color* output, int num_leds) {
    unsigned long time_since_start = current_time_micros - this->pattern_start_time_micros;
    if (time_since_start >= this->pattern_duration_micros) {
        time_since_start -= this->pattern_duration_micros;
        this->pattern_start_time_micros = current_time_micros;
    }

    const int color_index = time_since_start / this->time_per_color_micros;
    const pattern_color& current_color = rainbow_colors[color_index];
    const pattern_color& next_color = rainbow_colors[(color_index + 1) % NUM_RAINBOW_COLORS];
    const float t = (float)(time_since_start % this->time_per_color_micros) / this->time_per_color_micros;
    const pattern_color base_color = mixColor(current_color, next_color, t);

    const bool is_beat = features.isBeat();
    pattern_color final_color;

    if (this->should_react_to_beat) {
        if (is_beat) {
            this->time_of_last_beat = current_time_micros;
        }
        // Stateless exponential decay from 255 at time_of_last_beat.
        // tau = brightness_decay_micros / 3 gives ~5% residual after the old
        // "full-decay" duration — visually similar to the prior linear behaviour
        // but with a more natural curve.
        const unsigned long time_since_beat = current_time_micros - this->time_of_last_beat;
        const float tau_s = static_cast<float>(this->brightness_decay_micros) * 1e-6f / 3.0f;
        const float dt_s  = static_cast<float>(time_since_beat) * 1e-6f;
        float flash = 255.0f * std::exp(-dt_s / tau_s);
        if (flash < 0.0f) flash = 0.0f;
        final_color = pattern_color{base_color.red, base_color.green, base_color.blue,
                                    static_cast<uint8_t>(flash)};
    } else {
        final_color = base_color;
    }

    for (int i = 0; i < num_leds; ++i) {
        output[i] = final_color;
    }
}

} // namespace jellED
