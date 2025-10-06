#ifndef _PATTERN_ENGINE_H_
#define _PATTERN_ENGINE_H_

#include "pattern/patternType.h"
#include "pattern/pattern_colors.h"
#include "pattern/patternBlueprint.h"
#include "pattern/rainbowPattern.h"
#include "pattern.h"
#include "pUtils/IPlatformUtils.h"

namespace jellED {

class PatternEngine {
private:
    Pattern current_pattern;
    unsigned long current_time_micros;
    const int num_leds;
    PatternBlueprint** allPatternBlueprints;
    PatternBlueprint* currentPatternBlueprint;

    IPlatformUtils& pUtils;

    PatternBlueprint* type_to_blueprint(const PatternType patternType);

public:
    PatternEngine(IPlatformUtils &pUtils, const int num_leds, unsigned long pattern_duration_micros);
    PatternEngine(IPlatformUtils &pUtils, const int num_leds, unsigned long pattern_duration_micros, unsigned long brightness_decay_micros);
    ~PatternEngine();

    /**
     * Generates a new pattern. After a call to `generate_pattern`,
     * every previously generated pattern is deleted.
    */
    const Pattern& generate_pattern(bool is_beat);
    void turnOnReactToBeat();
    void turnOffReactToBeat();
};

} // namespace jellED

#endif
