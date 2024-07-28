#ifndef __RINGBUFFER_JELLED_H__
#define __RINGBUFFER_JELLED_H__

#include <stdint.h>

class Ringbuffer {
private:
    static constexpr uint8_t MAX_LEN = 3;
    uint16_t buffer[MAX_LEN];

public:
    Ringbuffer();
    uint16_t get(uint8_t index);
    void append(uint16_t entry);
};

#endif