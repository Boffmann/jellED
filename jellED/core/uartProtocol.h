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

// ── Self-synchronizing framer ────────────────────────────────────────────────
// Reassembles packets from a raw byte stream that may have lost or gained bytes
// (a UART glitch, or a stray single-byte event such as UART_BUTTON_PRESSED
// injected on the same line). Feed bytes one at a time; `feed` returns true
// exactly when a complete, checksum-valid packet has been assembled into `out`.
//
// Why this exists: reading the stream in fixed 7-byte chunks desyncs forever the
// moment a single byte is dropped or injected — every later chunk is misaligned
// by a fixed offset, the header/checksum never match, yet 7 bytes are consumed
// each time, so it can never realign on its own.
//
// The framer keeps a header byte at index 0 (discarding anything that cannot
// start a frame) and, on a checksum failure, drops only the leading byte and
// rescans the bytes it already holds — one of them may be the true header. A
// single corrupt/dropped byte therefore costs at most a packet or two of
// resync, never a permanent desync. Fixed buffer, no heap, hot-path safe.
struct UartFramer {
    uint8_t buffer[UART_PACKET_SIZE] = {};
    uint8_t length = 0; // bytes held; always in [0, UART_PACKET_SIZE) on return

    void reset() { length = 0; }

    bool feed(uint8_t byte, UartFeatures& out) {
        buffer[length++] = byte;

        // A frame can only begin with the header byte; drop anything else that
        // ends up at the front (covers the initial hunt and post-failure rescan).
        slideToHeader();
        if (length < UART_PACKET_SIZE) return false;

        // Full candidate frame, guaranteed to start with the header byte.
        if (uart_parse_packet(buffer, out)) {
            length = 0; // consumed cleanly
            return true;
        }

        // Header matched but checksum failed: corruption somewhere in the frame.
        // Drop ONLY the leading header byte and rescan the remaining six — one of
        // them may be the real header. (Dropping all seven is the bug above.)
        dropLeadingByte();
        slideToHeader();
        return false;
    }

private:
    void dropLeadingByte() {
        for (uint8_t i = 1; i < length; ++i) buffer[i - 1] = buffer[i];
        --length;
    }

    void slideToHeader() {
        while (length > 0 && buffer[0] != UART_PACKET_HEADER) dropLeadingByte();
    }
};

} // namespace jellED

#endif
