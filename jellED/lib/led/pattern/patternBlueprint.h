#ifndef _PATTERN_BLUEPRINT_H_
#define _PATTERN_BLUEPRINT_H_

#include "pattern.h"
#include "pattern/patternType.h"
#include "pattern/patternConfig.h"

class PatternBlueprint {
protected:
    PatternType pattern_type;
    int num_leds;
    // The colors representing the finally generated pattern for the moment
    pattern_color* colors;
    pattern_config config;
    long tracked_time_since_last_beat_micros;

    virtual void init() = 0;
public:
    PatternBlueprint(PatternType patternType, int num_leds);
    virtual ~PatternBlueprint();
    PatternType get_pattern_type();
    int get_num_leds();
    void update_config(const pattern_config& config);
    virtual const pattern_color& get_color(int index);
    virtual void update_pattern(long time_since_bea_microst) = 0;
};

#endif