#include "patternEngineConfig.h"

#include <time.h>
#include <string.h>

PatternEngineConfig::PatternEngineConfig() :
current_pattern{-1} {
    srand(time(NULL));
}

void PatternEngineConfig::update_config(const pattern_engine_config& config) {
    this->config = config;
}

int PatternEngineConfig::get_beats_per_pattern() {
    return this->config.beats_per_pattern;
}

const PatternType PatternEngineConfig::get_next_pattern_type() {
    this->current_pattern++;
    if (this->current_pattern >= NUM_PATTERN_TYPES) {
        this->current_pattern = 0;
    }
    return this->get_pattern_type(this->current_pattern);
}

const PatternType PatternEngineConfig::get_random_pattern_type() {
    this->current_pattern = rand() % 4;

    return this->get_pattern_type(this->current_pattern);
}

const PatternType PatternEngineConfig::get_pattern_type(int index) {
    if (index == 0) {
        return this->config.pattern1;
    }
    if (index == 1) {
        return this->config.pattern2;
    }
    if (index == 2) {
        return this->config.pattern3;
    }
    return this->config.pattern4;
}
