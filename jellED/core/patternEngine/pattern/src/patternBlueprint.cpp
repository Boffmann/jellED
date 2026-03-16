#include "patternBlueprint.h"

namespace jellED {

PatternBlueprint::PatternBlueprint(unsigned long startTime, PatternType patternType,
                                   unsigned long pattern_duration_micros)
    : pattern_type{patternType},
      pattern_duration_micros{pattern_duration_micros},
      pattern_start_time_micros{startTime},
      should_react_to_beat{true},
      time_of_last_beat{startTime} {
}

PatternType PatternBlueprint::get_pattern_type() const {
    return this->pattern_type;
}

void PatternBlueprint::shouldReactToBeat(bool should_react, unsigned long current_time_micros) {
    this->should_react_to_beat = should_react;
    this->time_of_last_beat = current_time_micros;
}

} // namespace jellED
