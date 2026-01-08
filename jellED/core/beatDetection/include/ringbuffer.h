#ifndef __RINGBUFFER_JELLED_H__
#define __RINGBUFFER_JELLED_H__

#include <stdint.h>

namespace jellED {

class Ringbuffer {
private:
    double* buffer;
    double* scratch_buffer;  // For calculating median
    double* tail;
    uint32_t head_position;
    uint32_t max_len;
    uint32_t current_size;

    uint32_t get_position(const uint32_t index) const;

public:
    Ringbuffer(uint32_t max_len, bool initializeScratchBuffer = false);
    ~Ringbuffer();
    void fill(double value);
    double get(uint32_t index) const;
    void append(double entry);
    void override_head_value(double value);
    void clear();
    uint32_t size() const;
    double min() const;
    double max() const;
    double median() const;
};

} // end namespace jellED

#endif
