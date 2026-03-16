#ifndef __RINGBUFFER_JELLED_H__
#define __RINGBUFFER_JELLED_H__

#include <stdint.h>

namespace jellED {

class Ringbuffer {
private:
    float* buffer;
    float* scratch_buffer;  // For calculating median
    float* tail;
    uint32_t head_position;
    uint32_t max_len;
    uint32_t current_size;

    uint32_t get_position(const uint32_t index) const;

public:
    Ringbuffer(uint32_t max_len, bool initializeScratchBuffer = false);
    ~Ringbuffer();
    void fill(float value);
    float get(uint32_t index) const;
    void append(float entry);
    void override_head_value(float value);
    void clear();
    uint32_t size() const;
    float min() const;
    float max() const;
    float median() const;
};

} // end namespace jellED

#endif
