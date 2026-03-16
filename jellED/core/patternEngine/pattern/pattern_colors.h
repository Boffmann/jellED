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

// Convert HSV [h: 0..360, s: 0..1, v: 0..1] to RGB [0..255].
// Brightness field is set to 255 (caller may override).
static inline pattern_color hsvToRgb(float h, float s, float v) {
    h = h - 360.0f * static_cast<int>(h / 360.0f); // wrap to [0, 360)
    int   i  = static_cast<int>(h / 60.0f) % 6;
    float f  = h / 60.0f - static_cast<int>(h / 60.0f);
    float p  = v * (1.0f - s);
    float q  = v * (1.0f - f * s);
    float t  = v * (1.0f - (1.0f - f) * s);
    float r, g, b;
    switch (i) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }
    return pattern_color{
        static_cast<uint8_t>(r * 255.0f),
        static_cast<uint8_t>(g * 255.0f),
        static_cast<uint8_t>(b * 255.0f),
        255
    };
}

// Overload operator<< for Google Test output formatting
inline std::ostream& operator<<(std::ostream& os, const pattern_color& color) {
    os << "pattern_color{R:" << unsigned(color.red) << ", G:" << unsigned(color.green) << ", B:" << unsigned(color.blue) << ", B:" << unsigned(color.brightness) << "}";
    return os;
}

} // namespace jellED

#endif
