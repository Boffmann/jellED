#ifndef _PATTERN_ENGINE_H_
#define _PATTERN_ENGINE_H_

#include "pattern/patternType.h"
#include "pattern/pattern_colors.h"
#include "pattern/patternBlueprint.h"
#include "pattern.h"

class PatternEngine {
private:
    Pattern current_pattern;
    const int num_leds;
    PatternBlueprint** allPatternBlueprints;
    PatternBlueprint* currentPatternBlueprint;
    unsigned long int last_beat_time;
    long time_since_last_beat;

    PatternBlueprint* type_to_blueprint(PatternType patternType);

    void generate_error_pattern(const String &error_msg) {
        Serial.println(error_msg);
        for (int i = 0; i < this->num_leds; i++) {
            this->current_pattern.set_color(rainbow_colors[RED_INDEX], i);
        }
    };

    void generate_rainbow_pattern(const String &error_msg) {
        Serial.println(error_msg);
        for (int i = 0; i < this->num_leds; i++) {
            this->current_pattern.set_color(rainbow_colors[i % 6], i);
        }
    };

public:
    PatternEngine(const int num_leds);
    ~PatternEngine();

    void start(PatternType patternType);
    /**
     * Generates a new pattern. After a call to `generate_pattern`,
     * every previously generated pattern is deleted.
    */
    const Pattern& generate_pattern(bool is_beat);
    void set_pattern_type(const PatternType patternType);
    //const PatternType get_pattern_type();

};

#endif