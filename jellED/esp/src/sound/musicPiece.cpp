#include "musicPiece.h"

#include <Arduino.h>
// #include "SPIFFS.h"
#include "soundconfig.h"

namespace jellED {

MusicPiece::MusicPiece()
    : buffer_ptr{0} {}

MusicPiece::~MusicPiece() {
    if (this->buffer != nullptr) {
        free(this->buffer);
    }
}

void MusicPiece::initialize() {
    // TODO SPIFFS No such file or directory
    // if (!SPIFFS.begin(true)) {
    //     Serial.println("An Error has occurred while mounting SPIFFS");
    //     return;
    // }
    
    // File file = SPIFFS.open("/test_music_buffer.txt");
    // if(!file){
    //     Serial.println("Failed to open file for reading");
    //     return;
    // }

    // size_t currlen = 0;
    // this->num_samples_in_file = file.size() / this->BYTES_PER_SAMPLE;
    
    // Serial.println(this->num_samples_in_file);
    // this->buffer = (int16_t*) malloc(sizeof(int16_t) * this->num_samples_in_file + 1);
    // Serial.println(sizeof(int16_t) * this->num_samples_in_file + 1);
    
    // while(currlen < this->num_samples_in_file) {
    //     int16_t sample = 0;
    //     sample |= ((int16_t) file.read());
    //     sample |= ((int16_t) file.read() << 8);
    //     this->buffer[currlen] = sample;
    //     currlen++;
    // }
    // buffer[currlen] = '\0';
    // Serial.println("");
    // Serial.println(this->buffer[0]);
    // Serial.println(this->buffer[1]);
    // Serial.println(this->buffer[2]);
    // Serial.println(this->buffer[currlen - 1]);
    // Serial.println(this->buffer[currlen - 2]);
    // Serial.println("");

    // file.close();
}

bool MusicPiece::read(AudioBuffer* buffer) {
    // Check if buffer is initialized
    if (this->buffer == nullptr) {
        // Initialize with dummy data or return false
        Serial.println("Error: MusicPiece buffer not initialized");
        return false;
    }
    
    size_t num_samples = 0;
    for (int i = 0; i < this->MAX_BUFFER_SIZE; ++i) {
        buffer->buffer[i] = this->buffer[this->buffer_ptr];
        num_samples++;
        buffer_ptr++;
        if (buffer_ptr >= num_samples_in_file) {
            buffer_ptr = 0;
        }
    }
    buffer->bytes_read = 64;
    buffer->num_samples = num_samples;
    buffer->samplingRate = SAMPLE_RATE;
    
    return true;
}

} // end namespace jellED
