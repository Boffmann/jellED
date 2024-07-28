#include "ringbuffer.h"

#include <stdexcept>
#include <stdlib.h>
#include <algorithm>
#include <limits>

Ringbuffer::Ringbuffer(uint16_t max_len)
: buffer{nullptr},
  tail{nullptr},
  head_position{0},
  max_len{max_len},
  current_size{0}
 {
    buffer = (double*) malloc(sizeof(double) * max_len);
    tail = &buffer[max_len - 1];
    for (int index = 0; index < max_len; ++index) {
        this->buffer[index] = 0.0;
    }
}

Ringbuffer::~Ringbuffer() {
    if (this->buffer != nullptr) {
        free(this->buffer);
    }
}

double Ringbuffer::get(uint16_t index) {
    if (index >= this->max_len) {
        throw std::invalid_argument("Index out of bounds for Ringbuffer get");
    }
    uint16_t get_position = (this->head_position + index) % this->max_len;
    return this->buffer[get_position];
}

void Ringbuffer::append(double entry) {
    this->head_position = (this->head_position + 1) % this->max_len;
    uint16_t tail_pos = (this->head_position + this->max_len - 1) % this->max_len;
    this->tail = &buffer[tail_pos];
    *this->tail = entry;
    if (current_size < max_len) {
        current_size++;
    }
}

uint16_t Ringbuffer::size() const {
    return current_size;
}

double Ringbuffer::min() const {
    if (current_size == 0) {
        return 0.0;
    }
    double min_val = std::numeric_limits<double>::max();
    for (uint16_t i = 0; i < current_size; ++i) {
        uint16_t pos = (head_position + i) % max_len;
        if (buffer[pos] < min_val) {
            min_val = buffer[pos];
        }
    }
    return min_val;
}

double Ringbuffer::max() const {
    if (current_size == 0) {
        return 0.0;
    }
    double max_val = std::numeric_limits<double>::lowest();
    for (uint16_t i = 0; i < current_size; ++i) {
        uint16_t pos = (head_position + i) % max_len;
        if (buffer[pos] > max_val) {
            max_val = buffer[pos];
        }
    }
    return max_val;
}
