#ifndef _AUDIOBUFFER_JELLED_H_
#define _AUDIOBUFFER_JELLED_H_

#include <stdint.h>
#include <cstddef>

namespace jellED {

#define I2S_DMA_BUF_LEN 64

typedef struct AudioBuffer {
    double buffer[I2S_DMA_BUF_LEN];
    size_t bytes_read;
    size_t num_samples;
    uint16_t samplingRate;
} AudioBuffer;

} // namespace jellED

#endif
