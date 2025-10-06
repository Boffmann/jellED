#include "patternEngine.h"

namespace jellED {

constexpr int NUM_PATTERNS = 2;

PatternEngine::PatternEngine(IPlatformUtils& pUtils, const int num_leds, unsigned long pattern_duration_micros)
    :   current_pattern{pUtils, num_leds},
        num_leds{num_leds},
        currentPatternBlueprint{nullptr},
        allPatternBlueprints{nullptr},
        pUtils(pUtils) {

    this->current_time_micros = pUtils.crono().currentTimeMicros();
    this->allPatternBlueprints = (PatternBlueprint**) malloc(sizeof(PatternBlueprint*) * NUM_PATTERNS);
    this->allPatternBlueprints[PatternTypeCast::to_index(PatternType::RAINBOW)] = new RainbowPattern(this->current_time_micros, num_leds, pattern_duration_micros);
    this->currentPatternBlueprint = this->type_to_blueprint(PatternType::RAINBOW);
};

PatternEngine::PatternEngine(IPlatformUtils& pUtils, const int num_leds, unsigned long pattern_duration_micros, unsigned long brightness_decay_micros)
    :   current_pattern{pUtils, num_leds},
        num_leds{num_leds},
        currentPatternBlueprint{nullptr},
        allPatternBlueprints{nullptr},
        pUtils(pUtils) {

    this->current_time_micros = pUtils.crono().currentTimeMicros();
    this->allPatternBlueprints = (PatternBlueprint**) malloc(sizeof(PatternBlueprint*) * NUM_PATTERNS);
    this->allPatternBlueprints[PatternTypeCast::to_index(PatternType::RAINBOW)] = new RainbowPattern(this->current_time_micros, num_leds, pattern_duration_micros, brightness_decay_micros);
    this->currentPatternBlueprint = this->type_to_blueprint(PatternType::RAINBOW);
};

PatternEngine::~PatternEngine() {
    if (this->allPatternBlueprints != nullptr) {
        delete this->allPatternBlueprints[0];
        free(this->allPatternBlueprints);
    }
};

PatternBlueprint* PatternEngine::type_to_blueprint(const PatternType patternType) {
    return this->allPatternBlueprints[PatternTypeCast::to_index(patternType)];
}

const Pattern& PatternEngine::generate_pattern(bool is_beat) {
    this->current_time_micros = pUtils.crono().currentTimeMicros();
    this->currentPatternBlueprint->update_pattern(is_beat, this->current_time_micros);

    for (int color_index = 0; color_index < this->num_leds; ++color_index) {
        this->current_pattern.set_color(this->currentPatternBlueprint->get_color(color_index), color_index);
    }

    return this->current_pattern;
}

void PatternEngine::turnOnReactToBeat() {
    this->currentPatternBlueprint->shouldReactToBeat(true, this->current_time_micros);
}

void PatternEngine::turnOffReactToBeat() {
    this->currentPatternBlueprint->shouldReactToBeat(false, this->current_time_micros);
}

} // end namespace jellED
