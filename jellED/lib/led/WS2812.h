#ifndef _WS2812_ESP32_LIB_h
#define _WS2812_ESP32_LIB_h

#include <stdint.h>
#include "esp32-hal.h"

typedef uint8_t ColorChannel;

class WS2812 {
private:
	const uint8_t bit_resolution = 24;

	uint16_t ledCounts;
	uint8_t pin;
	uint8_t brightness;
	uint8_t rmt_channel;
	
	rmt_data_t *led_data;
	rmt_obj_t* rmt_send = NULL;
	
	void setPixel(int index, ColorChannel red, ColorChannel green, ColorChannel blue);
	void writeOne(int index, uint8_t bit);
	void writeZero(int index, uint8_t bit);

public:
	WS2812(uint8_t output_pin, uint16_t led_count);
    ~WS2812();

	bool initialize();
	void setBrightness(uint8_t brightness);
	
	void setColorChannelsFor(int index, uint8_t red, uint8_t green, uint8_t blue);

	esp_err_t show();
};

#endif

