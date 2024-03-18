#include "coloredAmplitude.h"
#include "../pattern.h"

// constexpr pattern_color colors[NUM_LEDS] = {
//     pattern_color{0, 255, 0},
//     pattern_color{0, 255, 0},
//     pattern_color{0, 255, 0},
//     pattern_color{0, 255, 0},
//     pattern_color{0, 255, 0}
// };

ColoredAmplitude::ColoredAmplitude(RenderingType renderingType, int num_leds)
 : PatternBlueprint(PatternType::COLORED_AMPLITUDE, renderingType, num_leds) {
    this->colors = (pattern_color*) malloc(sizeof(pattern_color) * num_leds);
    for (int color_index = 0; color_index < num_leds / 2; ++color_index) {
        this->colors[color_index] = pattern_color{0, 255, 0};
    }
    for (int color_index = num_leds / 2; color_index < num_leds / 2 + num_leds / 4; ++color_index) {
        this->colors[color_index] = pattern_color{255, 255, 0};
    }
    for (int color_index = num_leds / 2 + num_leds / 4; color_index < num_leds; ++color_index) {
        this->colors[color_index] = pattern_color{255, 0, 0};
    }
}

void ColoredAmplitude::update_pattern(long time_since_beat) {}

const pattern_color& ColoredAmplitude::get_color(int index) {
    if (index >= this->num_leds) {
        Serial.println("Error: Colored Amplitude Index out of bounds");
        return colors[0];
    }
    return colors[index];
}