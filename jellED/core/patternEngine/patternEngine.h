#ifndef _PATTERN_ENGINE_H_
#define _PATTERN_ENGINE_H_

#include "audioFeatures.h"
#include "patternBlueprint.h"
#include "patternType.h"
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

    PatternBlueprint* type_to_blueprint(PatternType patternType);

public:
    PatternEngine(IPlatformUtils& pUtils, int num_leds, unsigned long pattern_duration_micros,
                  unsigned long brightness_decay_micros = 1000000UL);
    ~PatternEngine();

    const Pattern& generate_pattern(const AudioFeatures& features);
    void selectPattern(PatternType type);
    PatternType currentPattern() const;
    void turnOnReactToBeat();
    void turnOffReactToBeat();
};

} // namespace jellED

#endif
