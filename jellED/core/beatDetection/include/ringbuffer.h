#ifndef __RINGBUFFER_JELLED_H__
#define __RINGBUFFER_JELLED_H__

#include <stdint.h>

namespace jellED {

class Ringbuffer {
private:
    double* buffer;
    double* tail;
    uint16_t head_position;
    uint16_t max_len;
    uint16_t current_size;

public:
    Ringbuffer(uint16_t max_len);
    ~Ringbuffer();
    double get(uint16_t index);
    void append(double entry);
    uint16_t size() const;
    double min() const;
    double max() const;
};

} // end namespace jellED

#endif
