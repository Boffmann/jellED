#ifndef _PATTERN_BLUEPRINT_H_
#define _PATTERN_BLUEPRINT_H_

#include "audioFeatures.h"
#include "pattern_colors.h"
#include "patternType.h"

namespace jellED {

class PatternBlueprint {
protected:
    const PatternType pattern_type;
    const unsigned long pattern_duration_micros;
    unsigned long pattern_start_time_micros;
    bool should_react_to_beat;
    unsigned long time_of_last_beat;

public:
    PatternBlueprint(unsigned long startTime, PatternType patternType, unsigned long pattern_duration_micros);
    virtual ~PatternBlueprint() = default;

    PatternType get_pattern_type() const;
    void shouldReactToBeat(bool should_react, unsigned long current_time_micros);

    // Writes the current frame into `output[0..num_leds-1]`.
    virtual void update_pattern(const AudioFeatures& features, unsigned long current_time_micros,
                                pattern_color* output, int num_leds) = 0;
};

} // namespace jellED

#endif
