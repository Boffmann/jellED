#ifndef _PATTERN_ENGINE_PATTERN_H_
#define _PATTERN_ENGINE_PATTERN_H_

#include <stdint.h>
#include <Arduino.h>

class Pattern;

typedef struct t_color {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} pattern_color;

class PatternBuilder {
private:
    int added_colors;
    Pattern* pattern;

public:
    PatternBuilder(const int length);
    void add_color(const pattern_color color);
    Pattern* build();
};

class Pattern {
private:
    friend class PatternBuilder;
    Pattern(const int length);
    void add_color(const pattern_color color);

    int current_length;
    int max_length;
    pattern_color *colors = nullptr;
    static constexpr int RED_INDEX = 0;
    static constexpr int YELLOW_INDEX = 0;
    static constexpr int GREEN_INDEX = 0;
    static constexpr int CYAN_INDEX = 0;
    static constexpr int BLUE_INDEX = 0;
    static constexpr int MAGENTA_INDEX = 0;
    static constexpr pattern_color rainbow_colors[6] = {
        pattern_color{255, 0, 0},       // red
        pattern_color{255, 255, 0},     // yellow
        pattern_color{0, 255, 0},       // green
        pattern_color{0, 255, 255},     // cyan
        pattern_color{0, 0, 255},       // blue
        pattern_color{255, 0, 255},     // magenta
    };

public:
    ~Pattern();
    const int get_length();
    const pattern_color* get_color(const int color_index);

    static Pattern* generate_error_pattern(const int length, const String &error_msg) {
        Serial.println(error_msg);
        PatternBuilder patternBuilder(length);
        for (int i = 0; i < length; ++i) {
            patternBuilder.add_color(rainbow_colors[RED_INDEX]);
        }
        return patternBuilder.build();
    };

    static Pattern* generate_rainbow_pattern(const int length, const String &error_msg) {
        Serial.println(error_msg);
        PatternBuilder patternBuilder(length);
        for (int i = 0; i < length; ++i) {
            patternBuilder.add_color(rainbow_colors[i % 6]);
        }
        return patternBuilder.build();
    };
};

#endif