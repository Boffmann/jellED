#include "alternatingColors.h"
#include "pattern.h"

AlternatingColors::AlternatingColors(int num_leds) 
 : PatternBlueprint(PatternType::ALTERNATING_COLORS, num_leds) {
    this->config.palette_color1 = pattern_color{255, 0, 0}; // Red
    this->config.palette_color2 = pattern_color{0, 255, 0}; // Green
    this->config.palette_color3 = pattern_color{0, 0, 0}; // Ignored
    this->init();
}

void AlternatingColors::init() {
    for (int color_index = 0; color_index < this->num_leds; ++color_index) {
        if (color_index % 2 == 0) {
            this->colors[color_index] = this->config.palette_color1;
        } else {
            this->colors[color_index] = this->config.palette_color2;
        }
    }
}

pattern_color& AlternatingColors::get_counterpart_for(const pattern_color& color) {
    if (color.red == this->config.palette_color1.red &&
        color.green == this->config.palette_color1.green &&
        color.blue == this->config.palette_color1.blue) {
            return this->config.palette_color2;
        }
    return this->config.palette_color1;
}

void AlternatingColors::update_pattern(long time_since_beat_micros) {
    if (time_since_beat_micros < this->tracked_time_since_last_beat_micros) {
        for (int color_index = 0; color_index < this->num_leds; ++color_index) {
            this->colors[color_index] = this->get_counterpart_for(this->colors[color_index]);
        }
    }
    this->tracked_time_since_last_beat_micros = time_since_beat_micros;
}
