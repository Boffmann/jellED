#include "patternEngine.h"
#include <Arduino.h>
#include "pattern/coloredAmplitude.h"
#include "pattern/alternatingColors.h"

constexpr int NUM_PATTERNS = 2;

PatternEngine::PatternEngine(const int num_leds)
    :   current_pattern{num_leds},
        num_leds{num_leds},
        currentPatternBlueprint{nullptr},
        allPatternBlueprints{nullptr},
        last_beat_time_micros{micros()},
        time_since_last_beat_micros{-1},
        beats_with_current_pattern{0} {};

PatternEngine::~PatternEngine() {
    if (this->allPatternBlueprints != nullptr) {
        free(this->allPatternBlueprints[0]);
        free(this->allPatternBlueprints[1]);
        free(this->allPatternBlueprints);
    }
};

void PatternEngine::start(const pattern_engine_config& config) {
    this->config.update_config(config);
    printf("Got here");
    this->time_since_last_beat_micros = micros();
    this->allPatternBlueprints = (PatternBlueprint**) malloc(sizeof(PatternBlueprint) * NUM_PATTERNS);

    this->allPatternBlueprints[PatternTypeCast::to_index(PatternType::COLORED_AMPLITUDE)] = new ColoredAmplitude(num_leds);
    this->allPatternBlueprints[PatternTypeCast::to_index(PatternType::ALTERNATING_COLORS)] = new AlternatingColors(num_leds);
    this->currentPatternBlueprint = this->type_to_blueprint(this->config.get_next_pattern_type());
}

PatternBlueprint* PatternEngine::type_to_blueprint(const PatternType patternType) {
    return this->allPatternBlueprints[PatternTypeCast::to_index(patternType)];
}

const Pattern& PatternEngine::generate_pattern(bool is_beat) {
    if (this->time_since_last_beat_micros == -1) {
        generate_error_pattern("Pattern Engine must be started before generating pattern.");
        return this->current_pattern;
    }
    if (this->beats_with_current_pattern >= this->config.get_beats_per_pattern()) {
        this->currentPatternBlueprint = this->type_to_blueprint(this->config.get_next_pattern_type());
    }
    if (is_beat) {
        this->last_beat_time_micros = micros();
        this->beats_with_current_pattern++;
    }
    this->time_since_last_beat_micros = micros() - this->last_beat_time_micros;

    this->currentPatternBlueprint->update_pattern(this->time_since_last_beat_micros);

    for (int color_index = 0; color_index < this->num_leds; ++color_index) {
        this->current_pattern.set_color(this->currentPatternBlueprint->get_color(color_index), color_index);
    }

    return this->current_pattern;
}

void PatternEngine::update_config(const pattern_engine_config& pattern_engine_config) {
    this->config.update_config(pattern_engine_config);
    this->currentPatternBlueprint = this->type_to_blueprint(this->config.get_next_pattern_type());
}

void PatternEngine::update_pattern_color_config(const pattern_config& pattern_config) {
    for (int pattern_index = 0; pattern_index < NUM_PATTERNS; pattern_index++) {
        this->allPatternBlueprints[pattern_index]->update_config(pattern_config);
    }
}
