#ifndef _INMP441_ESP32_LIB_h
#define _INMP441_ESP32_LIB_h

#include <stdint.h>
#include <driver/i2s.h>

#define I2S_DMA_BUF_LEN 64

class INMP441 {
private:
    const uint8_t pin_sck;
    const uint8_t pin_ws;
    const uint8_t pin_sd;
    int16_t buffer[I2S_DMA_BUF_LEN];

public:
    INMP441(uint8_t ws_pin, uint8_t sd_pin, uint8_t sck_pin);
    ~INMP441();

    void initialize();
    int16_t* read(size_t* buffer_length, bool* buffer_ready);
};

#endif