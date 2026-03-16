#include "pattern.h"
#include <stdlib.h>

namespace jellED {

Pattern::Pattern(int length)
    : length_{length},
      colors_{(pattern_color*) malloc(sizeof(pattern_color) * length)} {
}

Pattern::~Pattern() {
    if (colors_ != nullptr) {
        free(colors_);
    }
}

void Pattern::set_color(const pattern_color& color, int index) {
    if (index < 0 || index >= length_) {
        return;
    }
    colors_[index] = color;
}

const pattern_color& Pattern::get_color(int index) const {
    if (index >= 0 && index < length_) {
        return colors_[index];
    }
    return colors_[0];
}

pattern_color* Pattern::data() {
    return colors_;
}

int Pattern::get_length() const {
    return length_;
}

} // end namespace jellED
