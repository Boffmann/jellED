#include "INMP441.h"

#include "soundconfig.h"
#include <Arduino.h>

namespace jellED {

#define I2S_PORT I2S_NUM_0

INMP441::INMP441(uint8_t ws_pin, uint8_t sd_pin, uint8_t sck_pin) 
: pin_ws{ws_pin}, pin_sd{sd_pin}, pin_sck{sck_pin}, buffer32{0} {
}

INMP441::~INMP441() {
    i2s_driver_uninstall(I2S_PORT);
}

void INMP441::initialize() {
    const i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = 0,
        .dma_buf_count = 4,
        .dma_buf_len = I2S_DMA_BUF_LEN,
        .use_apll = false
    };
    
    if (ESP_OK != i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL)) {
        Serial.println("i2s_driver_install: error");
    }

    const i2s_pin_config_t pin_config = {
        .bck_io_num = pin_sck,
        .ws_io_num = pin_ws,   
        .data_out_num = -1,
        .data_in_num = pin_sd
    };

    if (ESP_OK != i2s_set_pin(I2S_PORT, &pin_config)) {
        Serial.println("i2s_set_pin: error");
    }

    i2s_zero_dma_buffer(I2S_PORT);
    i2s_start(I2S_PORT);
}

bool INMP441::read(AudioBuffer* buffer) {
    esp_err_t result = i2s_read(I2S_PORT, &buffer32, sizeof(int8_t) * I2S_DMA_BUF_LEN * 4, &buffer->bytes_read, portMAX_DELAY);

    //Serial.print("Bytes read: ");
    // Serial.print(buffer->bytes_read);
    buffer->num_samples = buffer->bytes_read / 4;
    buffer->samplingRate = SAMPLE_RATE;
    for (int i = 0; i < buffer->num_samples; i++) {
        // Serial.println("");
        // Serial.print((char)this->buffer32[i * 4], BIN);
        // Serial.print(" ");
        // Serial.print((char)this->buffer32[i * 4+1], BIN);
        // Serial.print(" ");
        // Serial.print((char)this->buffer32[i * 4+2], BIN);
        // Serial.print(" ");
        // Serial.println((char)this->buffer32[i * 4+3], BIN);
        // Serial.println("");
        // Serial.println(this->buffer32[i * 3+2]);
        // Serial.println(this->buffer32[i * 4+3]);

        int8_t msb = buffer32[i * 4];
        int8_t upper_mid = buffer32[i * 4 + 1];
        int8_t lower_mid = buffer32[i * 4 + 2];
        int8_t lsb = buffer32[i * 4 + 3];
        int16_t raw = (((int32_t)lower_mid) << 8) + (((int32_t)upper_mid));// + (((int32_t)lower_mid));// + ((int32_t)msb);
        memcpy(&buffer->buffer[i], &raw, sizeof(raw));

        // buffer->buffer[i] = (int16_t) buffer32[i] >> 11;
        // Serial.println(buffer->buffer[i]);
        // Serial.println("");
    }

    // https://esp32.com/viewtopic.php?t=15185
    // for (int i = 0; i < buffer->num_samples; i++) {
    //     uint8_t mid = buffer32[i * 4 + 2];
    //     uint8_t msb = buffer32[i * 4 + 3];
    //     buffer->buffer[i] = (((uint32_t)msb) << 8) + ((uint32_t)mid);
    // }

    return result == ESP_OK;
}

} // end namespace jellED
