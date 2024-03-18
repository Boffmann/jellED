#include "pattern.h"
#include <stdlib.h>
#include <Arduino.h>

Pattern::Pattern(const int length)
    :   max_length{length} {
    this->colors = (pattern_color*) malloc(sizeof(pattern_color) * this->max_length);
}

Pattern::~Pattern(){
    if (this->colors != nullptr) {
        free(this->colors);
    }
}

void Pattern::set_color(const pattern_color color, const int color_index) {
    if (color_index > max_length) {
        for (int i = 0; i < this->max_length; ++i) {
            this->colors[i] = rainbow_colors[RED_INDEX];
        }
        return;
    }
    this->colors[color_index] = color;
}

const int Pattern::get_length() const {
    return this->max_length;
}

const pattern_color& Pattern::get_color(const int color_index) const {
    if (0 <= color_index && color_index < this->max_length) {
        return this->colors[color_index];
    }
    Serial.println("Added too many colors");
    return rainbow_colors[RED_INDEX];
}