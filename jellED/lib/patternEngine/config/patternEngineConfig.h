#ifndef _JELLED_PATTERN_ENGINE_CONFIG_H_
#define _JELLED_PATTERN_ENGINE_CONFIG_H_

#include "pattern/patternType.h"

constexpr int NUM_PATTERN_TYPES = 4;

typedef struct t_pattern_engine_config {
    PatternType pattern1;
    PatternType pattern2;
    PatternType pattern3;
    PatternType pattern4;
    int beats_per_pattern;
} pattern_engine_config;

class PatternEngineConfig {
private:
    pattern_engine_config config;
    int current_pattern;

    const PatternType get_pattern_type(int index);

public:
    PatternEngineConfig();

    void update_config(const pattern_engine_config& config);

    int get_beats_per_pattern();
    const PatternType get_next_pattern_type();
    const PatternType get_random_pattern_type();
};

#endif
