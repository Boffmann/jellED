#ifndef __WAV_FILE_INPUT_RASPI_JELLED_H__
#define __WAV_FILE_INPUT_RASPI_JELLED_H__

#include "sound/soundinput.h"

#include <string>

#include "AudioFile.h"

namespace jellED {

class WavFile : public SoundInput {
private:
    AudioFile<double> audioFile;
    int currentBufferLocation;

public:
    WavFile(std::string& path);
    ~WavFile();
    bool read(AudioBuffer* buffer);
};

} // namespace jellED

#endif
