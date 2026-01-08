#include "include/ringbuffer.h"

#include <stdexcept>
#include <stdlib.h>
#include <algorithm>
#include <limits>
#include <iostream>

namespace jellED {

Ringbuffer::Ringbuffer(uint32_t max_len, bool initializeScratchBuffer)
: buffer{nullptr},
  tail{nullptr},
  head_position{0},
  max_len{max_len},
  current_size{0}
 {
    buffer = (double*) malloc(sizeof(double) * max_len);
    tail = &buffer[max_len - 1];
    for (uint32_t index = 0; index < max_len; ++index) {
        this->append(0.0);
    }
    if (initializeScratchBuffer) {
        scratch_buffer = (double*) malloc(sizeof(double) * max_len);
    } else {
        scratch_buffer = nullptr;
    }
}

Ringbuffer::~Ringbuffer() {
    if (this->buffer != nullptr) {
        free(this->buffer);
    }
    if (this->scratch_buffer != nullptr) {
        free(this->scratch_buffer);
    }
}

uint32_t Ringbuffer::get_position(const uint32_t index) const {
    if (index >= this->current_size) {
        throw std::invalid_argument("Index out of bounds for Ringbuffer get_position");
    }
    if (current_size < max_len) {
        // Buffer not full yet, elements are stored from 0 to current_size-1
        return index;
    } else {
        // Buffer is full, oldest element is at head_position
        return (this->head_position + index) % this->max_len;
    }
}

void Ringbuffer::fill(double value) {
    for (uint32_t index = 0; index < max_len; ++index) {
        this->append(value);
    }
}

double Ringbuffer::get(uint32_t index) const {
    return this->buffer[get_position(index)];
}

void Ringbuffer::append(double entry) {
    this->head_position = (this->head_position + 1) % this->max_len;
    uint32_t tail_pos = (this->head_position + this->max_len - 1) % this->max_len;
    this->tail = &buffer[tail_pos];
    *this->tail = entry;
    
    if (current_size < max_len) {
        current_size++;
    }
}

void Ringbuffer::override_head_value(double value) {
    //this->buffer[this->head_position] = value;
    *this->tail = value;
}

void Ringbuffer::clear() {
    this->head_position = 0;
    this->current_size = 0;
    for (uint32_t index = 0; index < max_len; ++index) {
        this->buffer[index] = 0.0;
    }
}

uint32_t Ringbuffer::size() const {
    return current_size;
}

double Ringbuffer::min() const {
    if (current_size == 0) {
        return 0.0;
    }
    double min_val = std::numeric_limits<double>::max();
    for (uint32_t i = 0; i < current_size; ++i) {
        uint32_t pos = get_position(i);
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
    for (uint32_t i = 0; i < current_size; ++i) {
        uint32_t pos = get_position(i);
        if (buffer[pos] > max_val) {
            max_val = buffer[pos];
        }
    }
    return max_val;
}

double Ringbuffer::median() const {
    if (current_size == 0) {
        return 0.0;
    }
    
    // Copy to scratch buffer
    for (uint32_t i = 0; i < current_size; ++i) {
        scratch_buffer[i] = buffer[get_position(i)];
    }
    
    // Partial sort using nth_element logic (or simple insertion sort for small N)
    std::nth_element(scratch_buffer, scratch_buffer + current_size/2, 
                     scratch_buffer + current_size);
    
    double med = scratch_buffer[current_size / 2];
    if (current_size % 2 == 0) {
        // For even count, average the two middle values
        double max_lower = *std::max_element(scratch_buffer, 
                                              scratch_buffer + current_size/2);
        med = (med + max_lower) / 2.0;
    }
    return med;
}

} // end namespace jellED
