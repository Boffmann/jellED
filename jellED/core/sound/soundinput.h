#ifndef _SOUND_INPUT_JELLED_H_
#define _SOUND_INPUT_JELLED_H_

#include "audiobuffer.h"

namespace jellED {

class SoundInput {
public:
    SoundInput(){};
    virtual ~SoundInput(){}
    virtual bool read(AudioBuffer* buffer) = 0;
};

} // namespace jellED

#endif

