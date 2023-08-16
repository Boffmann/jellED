#include "musicPiece.h"

#include <Arduino.h>
#include "SPIFFS.h"

MusicPiece::MusicPiece()
    : buffer_ptr{0} {

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
    this->buffer_size = file.size() / sizeof(int16_t);
    Serial.println(this->buffer_size);
    this->buffer = (int16_t*) malloc(sizeof(int16_t) * this->buffer_size + 1);
    Serial.println(sizeof(int16_t) * this->buffer_size + 1);
    
    while(currlen < this->buffer_size -1) {
        int16_t sample = 0;
        sample |= ((int16_t) file.read() << 8);
        sample |= ((int16_t) file.read());
        this->buffer[currlen] = sample;
        currlen++;
    }
    buffer[currlen] = '\0';

    file.close();
}

MusicPiece::~MusicPiece() {
    if (this->buffer != nullptr) {
        free(this->buffer);
    }
}

bool MusicPiece::read(AudioBuffer* buffer) {
    for (int i = 0; i < 8; ++i) {
        buffer->buffer[i] = this->buffer[this->buffer_ptr];
        buffer_ptr++;
        if (buffer_ptr >= buffer_size) {
            buffer_ptr = 0;
        }
    }
    buffer->buffer_bytes = 64;
    buffer->num_samples = 8;
    buffer->samplingFrequency = 44100;
    
    return true;
}
