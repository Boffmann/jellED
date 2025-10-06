#ifndef __JELLED_DEFAULT_COLORS_H__
#define __JELLED_DEFAULT_COLORS_H__

#include <stdint.h>
#include <iostream>

namespace jellED {

typedef struct t_color {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t brightness;

    bool operator== (const t_color &c1) const {
        return (red == c1.red && green == c1.green && blue == c1.blue && brightness == c1.brightness);
    }
} pattern_color;

static constexpr pattern_color RED = pattern_color{255, 0, 0, 255};
static constexpr pattern_color YELLOW = pattern_color{255, 255, 0, 255};
static constexpr pattern_color GREEN = pattern_color{0, 255, 0, 255};
static constexpr pattern_color CYAN = pattern_color{0, 255, 255, 255};
static constexpr pattern_color BLUE = pattern_color{0, 0, 255, 255};
static constexpr pattern_color MAGENTA = pattern_color{255, 0, 255, 255};

static uint8_t mix(const uint8_t& start, const uint8_t& end, float t) {
    t = (t < 0.0f) ? 0.0f : (t > 1.0f) ? 1.0f : t;
    return static_cast<uint8_t>(start + (end - start) * t);
}

static pattern_color mixColor(const pattern_color& start_color, 
    const pattern_color& end_color, 
    float t) {
    return pattern_color{
        mix(start_color.red, end_color.red, t),
        mix(start_color.green, end_color.green, t),
        mix(start_color.blue, end_color.blue, t),
        mix(start_color.brightness, end_color.brightness, t)
    };
}

// Overload operator<< for Google Test output formatting
inline std::ostream& operator<<(std::ostream& os, const pattern_color& color) {
    os << "pattern_color{R:" << unsigned(color.red) << ", G:" << unsigned(color.green) << ", B:" << unsigned(color.blue) << ", B:" << unsigned(color.brightness) << "}";
    return os;
}

} // namespace jellED

#endif
