#ifndef _PATTERN_ALTERNATING_COLORS_H_
#define _PATTERN_ALTERNATING_COLORS_H_

#include "patternBlueprint.h"

class AlternatingColors : public PatternBlueprint {
public:
    AlternatingColors(int num_leds);
    ~AlternatingColors() {};
    void update_pattern(long time_since_beat_micros);

protected:
    void init();
};

#endif
