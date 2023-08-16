#ifndef _AUDIOBUFFER_JELLED_H_
#define _AUDIOBUFFER_JELLED_H_

#include <stdint.h>
#include <cstddef>

#define I2S_DMA_BUF_LEN 64

typedef struct AudioBuffer {
    int16_t buffer[I2S_DMA_BUF_LEN];
    size_t buffer_bytes;
    size_t num_samples;
    double samplingFrequency;
} AudioBuffer;

#endif