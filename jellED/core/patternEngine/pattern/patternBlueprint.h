#ifndef _PATTERN_BLUEPRINT_H_
#define _PATTERN_BLUEPRINT_H_

#include "pattern.h"
#include "pattern/patternType.h"
#include "pUtils/ILogger.h"

namespace jellED {

class PatternBlueprint {
protected:
    const PatternType pattern_type;
    const int num_leds;
    const unsigned long pattern_duration_micros;
    // The colors representing the finally generated pattern for the moment
    pattern_color* colors;
    unsigned long pattern_start_time_micros;
    bool should_react_to_beat;
    unsigned long time_of_last_beat;

    virtual void init() = 0;
public:
    PatternBlueprint(unsigned long startTime, PatternType patternType, int num_leds, unsigned long pattern_duration_micros);
    virtual ~PatternBlueprint();
    PatternType get_pattern_type();
    int get_num_leds();
    const pattern_color& get_color(int index);
    void shouldReactToBeat(bool should_react, unsigned long current_time_micros);
    
    virtual void update_pattern(bool is_beat, unsigned long current_time_micros) = 0;
};

} // namespace jellED

#endif
