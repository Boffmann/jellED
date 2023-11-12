#include "INMP441.h"

#include "soundconfig.h"
#include <Arduino.h>

#define I2S_PORT I2S_NUM_0

INMP441::INMP441(uint8_t ws_pin, uint8_t sd_pin, uint8_t sck_pin) 
: pin_ws{ws_pin}, pin_sd{sd_pin}, pin_sck{sck_pin} {}

INMP441::~INMP441() {
    i2s_driver_uninstall(I2S_PORT);
}

void INMP441::initialize() {
    const i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = 0,
        .dma_buf_count = 4,
        .dma_buf_len = I2S_DMA_BUF_LEN,
        .use_apll = false
    };
    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);

    const i2s_pin_config_t pin_config = {
        .bck_io_num = pin_sck,
        .ws_io_num = pin_ws,   
        .data_out_num = -1,
        .data_in_num = pin_sd
    };
    i2s_set_pin(I2S_PORT, &pin_config);

    i2s_zero_dma_buffer(I2S_PORT);
    i2s_start(I2S_PORT);
}

bool INMP441::read(AudioBuffer* buffer) {
    esp_err_t result = i2s_read(I2S_PORT, &buffer->buffer, sizeof(int16_t) * I2S_DMA_BUF_LEN, &buffer->buffer_bytes, portMAX_DELAY);
    buffer->num_samples = buffer->buffer_bytes / sizeof(int16_t);
    buffer->samplingRate = SAMPLE_RATE;
    return result == ESP_OK;
}