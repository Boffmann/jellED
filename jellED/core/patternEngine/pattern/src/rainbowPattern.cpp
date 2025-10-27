#include "rainbowPattern.h"
#include "pattern.h"
#include <algorithm>
#include <cmath>

namespace jellED {

static constexpr pattern_color rainbow_colors[] = {RED, YELLOW, GREEN, CYAN, BLUE, MAGENTA};
static constexpr unsigned long DEFAULT_BRIGHTNESS_DECAY_MICROS = 1000000;

RainbowPattern::RainbowPattern(unsigned long startTime, int num_leds, unsigned long pattern_duration_micros)
 : RainbowPattern(startTime, num_leds, pattern_duration_micros, DEFAULT_BRIGHTNESS_DECAY_MICROS) {
    this->init();
}

RainbowPattern::RainbowPattern(unsigned long startTime, int num_leds, unsigned long pattern_duration_micros, unsigned long brightness_decay_micros)
 : PatternBlueprint(startTime, PatternType::RAINBOW, num_leds, pattern_duration_micros),
 time_per_color_micros{pattern_duration_micros / (sizeof(rainbow_colors) / sizeof(*rainbow_colors))},
 brightness_decay_micros{brightness_decay_micros},
 current_color_index{0},
 current_brightness{255} {
    this->current_color_index = 0;
    this->should_react_to_beat = should_react_to_beat;
    this->current_brightness = 255;
    this->init();
}

void RainbowPattern::init() {
    for (int color_index = 0; color_index < num_leds; ++color_index) {
        this->colors[color_index] = RED;
    }
}

void RainbowPattern::update_pattern(bool is_beat, unsigned long current_time_micros) {
    unsigned long time_since_pattern_start_micros = current_time_micros - this->pattern_start_time_micros;
    if (time_since_pattern_start_micros >= this->pattern_duration_micros) {
        time_since_pattern_start_micros = time_since_pattern_start_micros - this->pattern_duration_micros;
        this->pattern_start_time_micros = current_time_micros;
    }
    this->current_color_index = time_since_pattern_start_micros / this->time_per_color_micros;
    const pattern_color current_color = rainbow_colors[this->current_color_index];
    const pattern_color next_color = rainbow_colors[(this->current_color_index + 1) % sizeof(rainbow_colors)];
    const float t = (float)(time_since_pattern_start_micros % this->time_per_color_micros) / this->time_per_color_micros;
    const pattern_color result_color = mixColor(current_color, next_color, t);

    for (int color_index = 0; color_index < num_leds; ++color_index) {
        if (this->should_react_to_beat) {
            const unsigned long time_since_beat_micros = current_time_micros - this->time_of_last_beat;
            const float brightness_decay_t = (time_since_beat_micros >= this->brightness_decay_micros)
            ? 1.0f
            : (float)(time_since_beat_micros % this->brightness_decay_micros) / this->brightness_decay_micros;
            const uint8_t brightness = is_beat ? 255 : mix(result_color.brightness, 0, brightness_decay_t);
            this->colors[color_index] = pattern_color{result_color.red, result_color.green, result_color.blue, brightness};
            if (is_beat) {
                this->time_of_last_beat = current_time_micros;
            }
        } else {
            this->colors[color_index] = result_color;
        }
    }
}

} // namespace jellED
