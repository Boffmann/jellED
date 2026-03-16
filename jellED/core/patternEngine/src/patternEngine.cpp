#include "patternEngine.h"
#include "rainbowPattern.h"
#include "breathingGlowPattern.h"
#include "pulseFlashPattern.h"
#include "sparklePattern.h"

#include <stdlib.h>

namespace jellED {

constexpr int NUM_PATTERNS = static_cast<int>(PatternType::_COUNT);

PatternEngine::PatternEngine(IPlatformUtils& pUtils, int num_leds,
                             unsigned long pattern_duration_micros,
                             unsigned long brightness_decay_micros)
    : current_pattern{num_leds},
      num_leds{num_leds},
      allPatternBlueprints{nullptr},
      currentPatternBlueprint{nullptr},
      pUtils(pUtils) {

    this->current_time_micros = pUtils.crono().currentTimeMicros();
    this->allPatternBlueprints = (PatternBlueprint**) malloc(sizeof(PatternBlueprint*) * NUM_PATTERNS);
    this->allPatternBlueprints[PatternTypeCast::to_index(PatternType::RAINBOW)] =
        new RainbowPattern(this->current_time_micros, pattern_duration_micros, brightness_decay_micros);
    this->allPatternBlueprints[PatternTypeCast::to_index(PatternType::BREATHING_GLOW)] =
        new BreathingGlowPattern(this->current_time_micros, pattern_duration_micros);
    this->allPatternBlueprints[PatternTypeCast::to_index(PatternType::PULSE_FLASH)] =
        new PulseFlashPattern(this->current_time_micros, pattern_duration_micros);
    this->allPatternBlueprints[PatternTypeCast::to_index(PatternType::SPARKLE)] =
        new SparklePattern(this->current_time_micros, num_leds, pattern_duration_micros);
    this->currentPatternBlueprint = this->type_to_blueprint(PatternType::RAINBOW);
}

PatternEngine::~PatternEngine() {
    if (this->allPatternBlueprints != nullptr) {
        for (int i = 0; i < NUM_PATTERNS; ++i) {
            delete this->allPatternBlueprints[i];
        }
        free(this->allPatternBlueprints);
    }
}

PatternBlueprint* PatternEngine::type_to_blueprint(PatternType patternType) {
    return this->allPatternBlueprints[PatternTypeCast::to_index(patternType)];
}

const Pattern& PatternEngine::generate_pattern(const AudioFeatures& features) {
    this->current_time_micros = pUtils.crono().currentTimeMicros();
    this->currentPatternBlueprint->update_pattern(
        features,
        this->current_time_micros,
        this->current_pattern.data(),
        this->current_pattern.get_length()
    );
    return this->current_pattern;
}

void PatternEngine::turnOnReactToBeat() {
    this->currentPatternBlueprint->shouldReactToBeat(true, this->current_time_micros);
}

void PatternEngine::selectPattern(PatternType type) {
    this->currentPatternBlueprint = this->type_to_blueprint(type);
}

PatternType PatternEngine::currentPattern() const {
    return this->currentPatternBlueprint->get_pattern_type();
}

void PatternEngine::turnOffReactToBeat() {
    this->currentPatternBlueprint->shouldReactToBeat(false, this->current_time_micros);
}

} // end namespace jellED
