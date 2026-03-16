#include "rainbowPattern.h"
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
        const unsigned long time_since_beat = current_time_micros - this->time_of_last_beat;
        const float decay_t = (time_since_beat >= this->brightness_decay_micros)
            ? 1.0f
            : (float)time_since_beat / this->brightness_decay_micros;
        const uint8_t brightness = is_beat ? 255 : mix(base_color.brightness, 0, decay_t);
        final_color = pattern_color{base_color.red, base_color.green, base_color.blue, brightness};
        if (is_beat) {
            this->time_of_last_beat = current_time_micros;
        }
    } else {
        final_color = base_color;
    }

    for (int i = 0; i < num_leds; ++i) {
        output[i] = final_color;
    }
}

} // namespace jellED
