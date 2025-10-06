#include "patternBlueprint.h"

#include <cstdlib>

namespace jellED {

PatternBlueprint::PatternBlueprint(unsigned long startTime, PatternType patternType, int num_leds, unsigned long pattern_duration_micros)
    : pattern_type{patternType},
    num_leds{num_leds},
    pattern_duration_micros{pattern_duration_micros},
    should_react_to_beat{true},
    time_of_last_beat{startTime},
    pattern_start_time_micros{startTime} {
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

int PatternBlueprint::get_num_leds() {
    return this->num_leds;
};

const pattern_color& PatternBlueprint::get_color(int index) {
    if (index >= this->num_leds) {
        return colors[0];
    }
    return colors[index];
}

void PatternBlueprint::shouldReactToBeat(bool should_react, unsigned long current_time_micros) {
    this->should_react_to_beat = should_react;
    this->time_of_last_beat = current_time_micros;
}

} // namespace jellED
