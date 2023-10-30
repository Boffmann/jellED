#include "musicPiece.h"

#include <Arduino.h>
#include "SPIFFS.h"
#include "soundconfig.h"

MusicPiece::MusicPiece()
    : buffer_ptr{0} {}

MusicPiece::~MusicPiece() {
    if (this->buffer != nullptr) {
        free(this->buffer);
    }
}

void MusicPiece::initialize() {
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }
    
    File file = SPIFFS.open("/test_music_buffer.txt");
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    size_t currlen = 0;
    this->num_samples_in_file = file.size() / this->BYTES_PER_SAMPLE;
    
    Serial.println(this->num_samples_in_file);
    this->buffer = (int16_t*) malloc(sizeof(int16_t) * this->num_samples_in_file + 1);
    Serial.println(sizeof(int16_t) * this->num_samples_in_file + 1);
    
    while(currlen < this->num_samples_in_file) {
        int16_t sample = 0;
        // sample |= ((int16_t) file.read() << 8);
        sample |= ((int16_t) file.read());
        this->buffer[currlen] = sample;
        currlen++;
    }
    buffer[currlen] = '\0';
    Serial.println(currlen);
    Serial.println(this->buffer[39999]);

    file.close();
}

bool MusicPiece::read(AudioBuffer* buffer) {
    size_t num_samples = 0;
    for (int i = 0; i < this->MAX_BUFFER_SIZE; ++i) {
        buffer->buffer[i] = this->buffer[this->buffer_ptr];
        num_samples++;
        buffer_ptr++;
        if (buffer_ptr >= num_samples_in_file) {
            buffer_ptr = 0;
        }
    }
    buffer->buffer_bytes = 64;
    buffer->num_samples = num_samples;
    buffer->samplingRate = SAMPLE_RATE;
    
    return true;
}
