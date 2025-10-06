#ifndef _PATTERN_ENGINE_PATTERN_H_
#define _PATTERN_ENGINE_PATTERN_H_

#include "pattern/pattern_colors.h"

#include "pUtils/IPlatformUtils.h"

namespace jellED {

class Pattern {
private:
    int max_length;
    pattern_color *colors = nullptr;
    IPlatformUtils& pUtils;

public:
    Pattern(IPlatformUtils& pUtils, const int length);
    void set_color(const pattern_color color, const int color_index);
    ~Pattern();
    const int get_length() const;
    const pattern_color& get_color(const int color_index) const;
};

} // namespace jellED

#endif