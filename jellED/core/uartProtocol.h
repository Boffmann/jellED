#ifndef __UART_PROTOCOL_H__
#define __UART_PROTOCOL_H__

#include <stdint.h>

namespace jellED {

// ── Packet layout (7 bytes) ──────────────────────────────────────────────────
//  Byte 0 : 0xAA          — sync header
//  Byte 1 : vol_low       — bass band volume  [0–255]
//  Byte 2 : vol_mid       — mid band volume   [0–255]
//  Byte 3 : vol_high      — treble band volume [0–255]
//  Byte 4 : beat_flags    — bit0=low beat, bit1=mid beat, bit2=high beat, bit3=any beat
//  Byte 5 : spectral_tilt — 0=treble-heavy, 128=balanced, 255=bass-heavy
//  Byte 6 : checksum      — XOR of bytes 1..5
// ─────────────────────────────────────────────────────────────────────────────
//
// NOTE: beat_flags bit values are intentionally identical to AudioFeatures::BEAT_*
// on the ESP side. Keep them in sync if either is changed.

constexpr uint8_t UART_PACKET_HEADER  = 0xAA;
constexpr uint8_t UART_PACKET_SIZE    = 7;
constexpr uint8_t UART_BUTTON_PRESSED = 0xFF; // legacy single-byte event

struct UartFeatures {
    static constexpr uint8_t BEAT_LOW   = 0x01;
    static constexpr uint8_t BEAT_MID   = 0x02;
    static constexpr uint8_t BEAT_HIGH  = 0x04;
    static constexpr uint8_t BEAT_FUSED = 0x08;

    uint8_t volumeLow    = 0;
    uint8_t volumeMid    = 0;
    uint8_t volumeHigh   = 0;
    uint8_t beatFlags    = 0;
    uint8_t spectralTilt = 128; // 0=treble-heavy, 128=balanced, 255=bass-heavy
};

inline void uart_build_packet(const UartFeatures& f, uint8_t out[UART_PACKET_SIZE]) {
    out[0] = UART_PACKET_HEADER;
    out[1] = f.volumeLow;
    out[2] = f.volumeMid;
    out[3] = f.volumeHigh;
    out[4] = f.beatFlags;
    out[5] = f.spectralTilt;
    out[6] = out[1] ^ out[2] ^ out[3] ^ out[4] ^ out[5];
}

inline bool uart_parse_packet(const uint8_t in[UART_PACKET_SIZE], UartFeatures& out) {
    if (in[0] != UART_PACKET_HEADER) return false;
    const uint8_t checksum = in[1] ^ in[2] ^ in[3] ^ in[4] ^ in[5];
    if (in[6] != checksum) return false;
    out.volumeLow    = in[1];
    out.volumeMid    = in[2];
    out.volumeHigh   = in[3];
    out.beatFlags    = in[4];
    out.spectralTilt = in[5];
    return true;
}

} // namespace jellED

#endif
