#include "coloredAmplitude.h"
#include "pattern.h"
#include <algorithm>
#include <cmath>

ColoredAmplitude::ColoredAmplitude(IPlatformUtils& pUtils, int num_leds)
 : PatternBlueprint(pUtils, PatternType::COLORED_AMPLITUDE, num_leds) {
    this->config.palette_color1 = pattern_color{0, 255, 0}; // Green
    this->config.palette_color2 = pattern_color{255, 255, 0}; // Yellow
    this->config.palette_color3 = pattern_color{255, 0, 0}; // Red
    this->init();
}

void ColoredAmplitude::init() {
    pUtils.logger().log("Init");
    if (num_leds == 1) {
        this->colors[0] = this->config.palette_color1;
        return;
    }
    if (num_leds == 2) {
        this->colors[0] = this->config.palette_color1;
        this->colors[1] = this->config.palette_color3;
        return;
    }
    if (num_leds == 3) {
        this->colors[0] = this->config.palette_color1;
        this->colors[1] = this->config.palette_color2;
        this->colors[2] = this->config.palette_color3;
        return;
    }

    for (int color_index = 0; color_index < num_leds / 2; ++color_index) {
        this->colors[color_index] = this->config.palette_color1;
    }
    for (int color_index = num_leds / 2; color_index < num_leds / 2 + std::ceil((double)num_leds / 4); ++color_index) {
        this->colors[color_index] = this->config.palette_color2;
    }
    for (int color_index = num_leds / 2 + std::ceil((double)num_leds / 4); color_index < num_leds; ++color_index) {
        this->colors[color_index] = this->config.palette_color3;
    }
}

void ColoredAmplitude::update_pattern(long time_since_beat_micros) {
    const long micros_until_off = 500000;
    if (this->tracked_time_since_last_beat_micros >= time_since_beat_micros) {
        this->tracked_time_since_last_beat_micros = time_since_beat_micros;
        init();
        return;
    }
    this->tracked_time_since_last_beat_micros = time_since_beat_micros;
    const int turn_off_amount = (double)num_leds * std::min(1.0, (double)time_since_beat_micros / (double)micros_until_off);
    
    for (int turn_off = 1; turn_off <= turn_off_amount; ++turn_off) {
        this->colors[num_leds - turn_off] = pattern_color{0, 0, 0};
    }
}
