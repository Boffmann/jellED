#include "patternBlueprint.h"

#include <cstdlib>

PatternBlueprint::PatternBlueprint(IPlatformUtils& pUtils, PatternType patternType, int num_leds)
    : pattern_type{patternType},
    num_leds{num_leds},
    tracked_time_since_last_beat_micros{0},
    pUtils(pUtils) {

    this->colors = (pattern_color*) malloc(sizeof(pattern_color) * num_leds);
}

PatternBlueprint::~PatternBlueprint() {
    if (this->colors != nullptr) {
        free(colors);
    }
}

PatternType PatternBlueprint::get_pattern_type() {
    return this->pattern_type;
}

void PatternBlueprint::update_config(const pattern_config& config) {
    this->config = config;
    this->init();
}

int PatternBlueprint::get_num_leds() {
    return this->num_leds;
};

const pattern_color& PatternBlueprint::get_color(int index) {
    if (index >= this->num_leds) {
        pUtils.logger().log("Error: Colored Amplitude Index out of bounds");
        return colors[0];
    }
    return colors[index];
}