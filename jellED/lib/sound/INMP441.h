#ifndef _INMP441_ESP32_LIB_h
#define _INMP441_ESP32_LIB_h

#include "soundinput.h"

#include <stdint.h>
#include <driver/i2s.h>
#include "audiobuffer.h"

class INMP441 : public SoundInput {
private:
    const uint8_t pin_sck;
    const uint8_t pin_ws;
    const uint8_t pin_sd;

public:
    INMP441(uint8_t ws_pin, uint8_t sd_pin, uint8_t sck_pin);
    ~INMP441();

    void initialize();
    bool read(AudioBuffer* buffer);
};

#endif