#ifndef _AUDIO_FEATURES_H_
#define _AUDIO_FEATURES_H_

#include <stdint.h>

namespace jellED {

struct AudioFeatures {
    uint8_t volumeLow    = 0;
    uint8_t volumeMid    = 0;
    uint8_t volumeHigh   = 0;
    uint8_t beatFlags    = 0;
    uint8_t spectralTilt = 128; // 0 = bass-heavy, 255 = treble-heavy

    static constexpr uint8_t BEAT_LOW   = 0x01;
    static constexpr uint8_t BEAT_MID   = 0x02;
    static constexpr uint8_t BEAT_HIGH  = 0x04;
    static constexpr uint8_t BEAT_FUSED = 0x08;

    bool isBeat()      const { return beatFlags != 0; }
    bool isBeatLow()   const { return (beatFlags & BEAT_LOW)   != 0; }
    bool isBeatMid()   const { return (beatFlags & BEAT_MID)   != 0; }
    bool isBeatHigh()  const { return (beatFlags & BEAT_HIGH)  != 0; }
    bool isBeatFused() const { return (beatFlags & BEAT_FUSED) != 0; }
};

} // namespace jellED

#endif
