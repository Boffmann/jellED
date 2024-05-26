#ifndef _PATTERN_ENGINE_H_
#define _PATTERN_ENGINE_H_

#include "pattern/patternType.h"
#include "patternEngineConfig.h"
#include "pattern/pattern_colors.h"
#include "pattern/patternBlueprint.h"
#include "pattern.h"

class PatternEngine {
private:
    Pattern current_pattern;
    const int num_leds;
    PatternBlueprint** allPatternBlueprints;
    PatternBlueprint* currentPatternBlueprint;
    PatternEngineConfig config;
    unsigned long int last_beat_time_micros;
    long time_since_last_beat_micros;
    long beats_with_current_pattern;

    PatternBlueprint* type_to_blueprint(const PatternType patternType);

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

    void start(const pattern_engine_config& config);
    /**
     * Generates a new pattern. After a call to `generate_pattern`,
     * every previously generated pattern is deleted.
    */
    const Pattern& generate_pattern(bool is_beat);
    void update_config(const pattern_engine_config& pattern_engine_config);
    void update_pattern_color_config(const pattern_config& pattern_config);

};

#endif