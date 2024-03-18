#include "patternBlueprint.h"

PatternBlueprint::PatternBlueprint(PatternType patternType, RenderingType renderingType, int num_leds)
    : pattern_type{patternType},
    rendering_type{renderingType},
    num_leds{num_leds},
    colors{nullptr} {}

PatternBlueprint::~PatternBlueprint() {
    if (this->colors != nullptr) {
        delete colors;
    }
}

PatternType PatternBlueprint::get_pattern_type() {
    return this->pattern_type;
}

RenderingType PatternBlueprint::get_rendering_type() {
    return this->rendering_type;
}

int PatternBlueprint::get_num_leds() {
    return this->num_leds;
};