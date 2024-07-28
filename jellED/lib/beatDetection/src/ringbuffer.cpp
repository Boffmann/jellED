#include "ringbuffer.h"

Ringbuffer::Ringbuffer() {
    for (int index = 0; index < MAX_LEN; ++index) {
        this->buffer[index] = 0;
    }
}

uint16_t Ringbuffer::get(uint8_t index) {
    return this->buffer[index];
}

void Ringbuffer::append(uint16_t entry) {
    for (uint8_t index = 0; index < MAX_LEN - 1; ++index) {
        this->buffer[index] = this->buffer[index + 1];
    }
    this->buffer[MAX_LEN - 1] = entry;
}