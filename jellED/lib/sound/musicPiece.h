#ifndef _MUSIC_PIECE_JELLED_TEST_H_
#define _MUSIC_PIECE_JELLED_TEST_H_
#include "soundinput.h"

#include "audiobuffer.h"

class MusicPiece : public SoundInput {
private:
    size_t buffer_ptr;
    int16_t* buffer = nullptr;
    size_t buffer_size;

public:
    MusicPiece();
    ~MusicPiece();
    bool read(AudioBuffer* buffer);

};

#endif