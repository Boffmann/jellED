#ifndef _SPEAKER_JELLED_H_
#define _SPEAKER_JELLED_H_

#include <stdint.h>

namespace jellED {

class Speaker {
private:
    uint16_t sample_rate;
    const long OUTPUT_DELAY_MICRO;
    //  = 1000000 / SAMPLE_RATE;
    long last_output_time;

public:
    Speaker(const uint16_t sample_rate);

    void initialize();
    void play(const uint8_t audio);
};

} // namespace jellED

#endif
