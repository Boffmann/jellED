#include "pattern.h"
#include <stdlib.h>
#include <Arduino.h>

constexpr pattern_color Pattern::rainbow_colors[6];

Pattern::Pattern(const int length)
    :   current_length{0},
        max_length{length} {
    this->colors = (pattern_color*) malloc(sizeof(pattern_color) * this->max_length);
}

Pattern::~Pattern(){
    if (this->colors != nullptr) {
        free(this->colors);
    }
}

void Pattern::add_color(const pattern_color color) {
    this->colors[current_length] = color;
    current_length++;
}

const int Pattern::get_length() {
    return this->max_length;
}

const pattern_color* Pattern::get_color(const int color_index) {
    if (0 <= color_index && color_index < this->max_length) {
        return &this->colors[color_index];
    }
    Serial.println("Added too many colors");
    return &rainbow_colors[RED_INDEX];
}

PatternBuilder::PatternBuilder(const int length)
:   added_colors{0} {
    pattern = new Pattern(length);
}

void PatternBuilder::add_color(const pattern_color color) {
    this->added_colors++;
    if (this->added_colors > this->pattern->max_length) {
        this->pattern = Pattern::generate_error_pattern(this->pattern->max_length, "Added too many colors");
        return;
    }
    this->pattern->add_color(color);
}

Pattern* PatternBuilder::build() {
    if (this->added_colors != this->pattern->max_length) {
        this->pattern = Pattern::generate_error_pattern(this->pattern->max_length, "Not added equal colors");
    }
    return pattern;
}