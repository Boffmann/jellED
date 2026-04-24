#ifndef __JELLED_QUANTIZE_H__
#define __JELLED_QUANTIZE_H__

#include <algorithm>
#include <cstdint>

namespace jellED {

// Volume quantization scale: maps envelope amplitude [0, 0.5] → [0, 255].
// Tune if LEDs are too dim (lower) or always saturate (raise). Shared between
// raspi (ESP UART producer) and GUI visualizer so both see identical
// post-quantization features.
static constexpr float VOLUME_SCALE = 512.0f;

// Quantise float volume [0, ∞) to [0, 255] using VOLUME_SCALE.
inline uint8_t quantizeVolume(float v) {
    return static_cast<uint8_t>(std::min(255.0f, std::max(0.0f, v * VOLUME_SCALE)));
}

// Quantise spectral tilt [-1, 1] to [0, 255]: 0=treble-heavy, 255=bass-heavy.
inline uint8_t quantizeTilt(float tilt) {
    return static_cast<uint8_t>(std::min(255.0f, std::max(0.0f, (tilt + 1.0f) * 127.5f)));
}

} // namespace jellED

#endif
