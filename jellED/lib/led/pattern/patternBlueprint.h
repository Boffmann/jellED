#ifndef _PATTERN_BLUEPRINT_H_
#define _PATTERN_BLUEPRINT_H_

#include "pattern.h"
#include "pattern/patternType.h"

class PatternBlueprint {
protected:
    PatternType pattern_type;
    int num_leds;
    pattern_color* colors;
    long tracked_time_since_last_beat;
public:
    PatternBlueprint(PatternType patternType, int num_leds);
    virtual ~PatternBlueprint();
    PatternType get_pattern_type();
    int get_num_leds();
    virtual void update_pattern(long time_since_beat) = 0;
    virtual const pattern_color& get_color(int index) = 0;
};

#endif