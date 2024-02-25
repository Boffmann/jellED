#ifndef _PATTERN_ENGINE_H_
#define _PATTERN_ENGINE_H_

#include "pattern/patternType.h"
#include "pattern.h"

class PatternEngine {
private:

    void delete_pattern();
    Pattern *current_pattern;
    const int num_leds;
    PatternType patternType;
    long time_since_last_beat;

public:
    PatternEngine(const int num_leds, PatternType patternType);
    ~PatternEngine();

    void start();
    /**
     * Generates a new pattern. After a call to `generate_pattern`,
     * every previously generated pattern is deleted.
    */
    Pattern* generate_pattern(bool is_beat);
    void set_pattern_type(const PatternType patternType);
    const PatternType get_pattern_type();

};

#endif