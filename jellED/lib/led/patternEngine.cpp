#include "patternEngine.h"
#include <Arduino.h>

PatternEngine::PatternEngine(const int num_leds, PatternType patternType)
    :   current_pattern{nullptr},
        num_leds{num_leds},
        patternType{patternType},
        time_since_last_beat{0} {};

PatternEngine::~PatternEngine() {};

void PatternEngine::delete_pattern() {
    if (this->current_pattern != nullptr) {
        delete this->current_pattern;
    }
}

void PatternEngine::start() {
    this->time_since_last_beat = micros();
}

Pattern* PatternEngine::generate_pattern(bool is_beat) {
    this->delete_pattern();
    if (this->time_since_last_beat == 0) {
        return Pattern::generate_error_pattern(num_leds, "Pattern Engine must be started before generating pattern.");
    }
    // TODO Pattern generation
    this->current_pattern = Pattern::generate_rainbow_pattern(num_leds, "Test");
    return this->current_pattern;
}

void PatternEngine::set_pattern_type(const PatternType patternType) {
    this->patternType = patternType;
}

const PatternType PatternEngine::get_pattern_type() {
    return this->patternType;
}