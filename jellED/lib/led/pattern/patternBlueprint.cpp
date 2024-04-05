#include "patternBlueprint.h"

PatternBlueprint::PatternBlueprint(PatternType patternType, int num_leds)
    : pattern_type{patternType},
    num_leds{num_leds},
    tracked_time_since_last_beat{0},
    colors{nullptr} {}

PatternBlueprint::~PatternBlueprint() {
    if (this->colors != nullptr) {
        delete colors;
    }
}

PatternType PatternBlueprint::get_pattern_type() {
    return this->pattern_type;
}

int PatternBlueprint::get_num_leds() {
    return this->num_leds;
};