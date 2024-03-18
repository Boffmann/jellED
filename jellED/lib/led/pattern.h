#ifndef _PATTERN_ENGINE_PATTERN_H_
#define _PATTERN_ENGINE_PATTERN_H_

#include <Arduino.h>
#include "pattern/pattern_colors.h"

class Pattern {
private:
    int max_length;
    pattern_color *colors = nullptr;

public:
    Pattern(const int length);
    void set_color(const pattern_color color, const int color_index);
    ~Pattern();
    const int get_length() const;
    const pattern_color& get_color(const int color_index) const;
};

#endif