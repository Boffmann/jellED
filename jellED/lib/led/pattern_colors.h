#ifndef __JELLED_DEFAULT_COLORS_H__
#define __JELLED_DEFAULT_COLORS_H__

typedef struct t_color {
    int red;
    int green;
    int blue;
} pattern_color;

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

#endif