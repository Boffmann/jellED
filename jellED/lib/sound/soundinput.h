#include "audiobuffer.h"

class SoundInput {
public:
    SoundInput(){};
    virtual ~SoundInput(){}
    virtual bool read(AudioBuffer* buffer) = 0;
};