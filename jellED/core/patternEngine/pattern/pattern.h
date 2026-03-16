#ifndef _PATTERN_ENGINE_PATTERN_H_
#define _PATTERN_ENGINE_PATTERN_H_

#include "pattern_colors.h"

namespace jellED {

class Pattern {
private:
    int length_;
    pattern_color* colors_;

public:
    Pattern(int length);
    ~Pattern();

    Pattern(const Pattern&) = delete;
    Pattern& operator=(const Pattern&) = delete;

    void set_color(const pattern_color& color, int index);
    const pattern_color& get_color(int index) const;
    pattern_color* data();
    int get_length() const;
};

} // namespace jellED

#endif
