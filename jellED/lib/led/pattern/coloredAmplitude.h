#ifndef _PATTERN_COLORED_AMPLITUDE_H_
#define _PATTERN_COLORED_AMPLITUDE_H_

#include "patternBlueprint.h"

class ColoredAmplitude : public PatternBlueprint {
public:
    ColoredAmplitude(int num_leds);
    ~ColoredAmplitude() {};
    void update_pattern(long time_since_beat);
    const pattern_color& get_color(int index);

private:
    void init();
};

#endif
