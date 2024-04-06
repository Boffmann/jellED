#include "patternEngine.h"
#include <Arduino.h>
#include "pattern/coloredAmplitude.h"
#include "pattern/alternatingColors.h"

constexpr int NUM_PATTERNS = 2;
constexpr int COLORED_AMPLITUDE_INDEX = 0;
constexpr int ALTERNATING_COLORS_INDEX = 1;

PatternEngine::PatternEngine(const int num_leds)
    :   current_pattern{num_leds},
        num_leds{num_leds},
        currentPatternBlueprint{nullptr},
        allPatternBlueprints{nullptr},
        last_beat_time_micros{micros()},
        time_since_last_beat_micros{-1} {};

PatternEngine::~PatternEngine() {};

void PatternEngine::start(PatternType patternType) {
    this->time_since_last_beat_micros = micros();
    this->allPatternBlueprints = (PatternBlueprint**) malloc(sizeof(PatternBlueprint) * NUM_PATTERNS);

    this->allPatternBlueprints[PatternTypeCast::to_index(PatternType::COLORED_AMPLITUDE)] = new ColoredAmplitude(num_leds);
    this->allPatternBlueprints[PatternTypeCast::to_index(PatternType::ALTERNATING_COLORS)] = new AlternatingColors(num_leds);

    this->currentPatternBlueprint = this->type_to_blueprint(patternType);
}

PatternBlueprint* PatternEngine::type_to_blueprint(PatternType patternType) {
    return this->allPatternBlueprints[PatternTypeCast::to_index(patternType)];
}

const Pattern& PatternEngine::generate_pattern(bool is_beat) {
    if (this->time_since_last_beat_micros == -1) {
        generate_error_pattern("Pattern Engine must be started before generating pattern.");
        return this->current_pattern;
    }
    if (is_beat) {
        this->last_beat_time_micros = micros();
    }
    this->time_since_last_beat_micros = micros() - this->last_beat_time_micros;

    this->currentPatternBlueprint->update_pattern(this->time_since_last_beat_micros);

    for (int color_index = 0; color_index < this->num_leds; ++color_index) {
        this->current_pattern.set_color(this->currentPatternBlueprint->get_color(color_index), color_index);
    }

    return this->current_pattern;
}

void PatternEngine::set_pattern_type(const PatternType patternType) {
    this->currentPatternBlueprint = type_to_blueprint(patternType);
}
