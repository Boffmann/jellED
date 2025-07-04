#include "sound/wavFile.h"

#include <iostream>
#include <cmath>

namespace jellED {

WavFile::WavFile(std::string& filepath) 
: currentBufferLocation{0}
{
    this->audioFile.load(filepath);
}

WavFile::~WavFile() {
}

bool WavFile::read(AudioBuffer* buffer) {
    if (this->currentBufferLocation >= this->audioFile.getNumSamplesPerChannel()) {
        return false;
    }
    int numSamples = 0;
    
    for (int i = 0; i < I2S_DMA_BUF_LEN; i++) {
        this->currentBufferLocation+=1;
        if (this->currentBufferLocation >= this->audioFile.getNumSamplesPerChannel()) {
            break;
        }
        numSamples++;
        buffer->buffer[i] = audioFile.samples[0][this->currentBufferLocation];
    }
    buffer->num_samples = numSamples;
    buffer->bytes_read = numSamples * 2;
    buffer->samplingRate = this->audioFile.getSampleRate();
    return true;
}

} // namespace jellED
