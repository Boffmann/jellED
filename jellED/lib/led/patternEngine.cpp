#include "patternEngine.h"
#include <Arduino.h>
#include "pattern/coloredAmplitude.h"

constexpr int NUM_PATTERNS = 1;
constexpr int COLORED_AMPLITUDE_INDEX = 0;

PatternEngine::PatternEngine(const int num_leds)
    :   current_pattern{num_leds},
        num_leds{num_leds},
        currentPatternBlueprint{nullptr},
        allPatternBlueprints{nullptr},
        last_beat_time{micros()},
        time_since_last_beat{-1} {};

PatternEngine::~PatternEngine() {};

void PatternEngine::start(PatternType patternType) {
    this->time_since_last_beat = micros();
    this->allPatternBlueprints = (PatternBlueprint**) malloc(sizeof(PatternBlueprint) * NUM_PATTERNS);
    this->allPatternBlueprints[COLORED_AMPLITUDE_INDEX] = new ColoredAmplitude(num_leds);
    this->currentPatternBlueprint = type_to_blueprint(patternType);
}

pattern_color color_lerp(const pattern_color &color1,
const pattern_color &color2, const float t) {
    return pattern_color{
        color1.red + (int)((color2.red - color1.red) * t),
        color1.green + (int)((color2.green - color1.green) * t),
        color1.blue + (int)((color2.blue - color1.blue) * t)
        };
}

PatternBlueprint* PatternEngine::type_to_blueprint(PatternType patternType) {
    switch (patternType) {
        case PatternType::COLORED_AMPLITUDE:
            return this->allPatternBlueprints[COLORED_AMPLITUDE_INDEX];
            break;
    }
}

const Pattern& PatternEngine::generate_pattern(bool is_beat) {
    if (this->time_since_last_beat == -1) {
        generate_error_pattern("Pattern Engine must be started before generating pattern.");
        return this->current_pattern;
    }
    if (is_beat) {
        this->last_beat_time = micros();
    }
    this->time_since_last_beat = micros() - this->last_beat_time;

    this->currentPatternBlueprint->update_pattern(this->time_since_last_beat);

    for (int color_index = 0; color_index < this->num_leds; ++color_index) {
        this->current_pattern.set_color(this->currentPatternBlueprint->get_color(color_index), color_index);
    }

    return this->current_pattern;
}

void PatternEngine::set_pattern_type(const PatternType patternType) {
    // delete this->patternBlueprint;
    this->currentPatternBlueprint = type_to_blueprint(patternType);
}
