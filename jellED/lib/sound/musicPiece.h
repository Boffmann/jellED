#ifndef _MUSIC_PIECE_JELLED_TEST_H_
#define _MUSIC_PIECE_JELLED_TEST_H_
#include "soundinput.h"

#include "audiobuffer.h"

class MusicPiece : public SoundInput {
private:
    const uint8_t MAX_BUFFER_SIZE = 8;
    const uint8_t BYTES_PER_SAMPLE = 1;
    const uint16_t SAMPLE_RATE = 8000;
    size_t buffer_ptr;
    int16_t* buffer = nullptr;
    size_t num_samples_in_file;

public:
    MusicPiece();
    ~MusicPiece();
    void initialize();
    bool read(AudioBuffer* buffer);
};

#endif