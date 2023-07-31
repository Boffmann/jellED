#include "WS2812.h"

WS2812::WS2812(uint8_t pin_gpio, uint16_t led_count) {
    led_data = new rmt_data_t[24*led_count];
	ledCounts = led_count;
	pin = pin_gpio;
	brightness = 255;
}

WS2812::~WS2812() {
	delete led_data;
	led_data = NULL;
}

bool WS2812::initialize() {
	if ((rmt_send = rmtInit(pin, RMT_TX_MODE, RMT_MEM_64)) == NULL){
		return false;
	}
	for(int index = 0; index < ledCounts; index++)
	{
		for (int bit = 0; bit < bit_resolution; bit++) {
			writeZero(index, bit);
		}
	}

	rmtSetTick(rmt_send, 100);
	return true;
}

void WS2812::setBrightness(uint8_t brightness) {
	this->brightness = constrain(brightness, 0, 255);
}

void WS2812::setColorChannelsFor(int index, ColorChannel red, ColorChannel green, ColorChannel blue) {
	uint8_t r = red * this->brightness / 255;
	uint8_t g = green * this->brightness / 255;
	uint8_t b = blue * this->brightness / 255;

	setPixel(index, g, r, b);
}

void WS2812::setPixel(int index, ColorChannel red, ColorChannel green, ColorChannel blue) {
	uint32_t color = red << 16 | green << 8 | blue ;
	for (uint8_t bit = 0; bit < bit_resolution; bit++) {
		if (color & (1 << (23-bit))) {
			writeOne(index, bit);
		} else {
			writeZero(index, bit);
		}
	}
}

void WS2812::writeOne(int index, uint8_t bit) {
	const uint8_t bit_index = index * bit_resolution + bit; 
	led_data[bit_index].level0 = 1;
	led_data[bit_index].duration0 = 7;
	led_data[bit_index].level1 = 0;
	led_data[bit_index].duration1 = 6;
}

void WS2812::writeZero(int index, uint8_t bit) {
	const uint8_t bit_index = index * bit_resolution + bit; 
	led_data[bit_index].level0 = 1;
	led_data[bit_index].duration0 = 4;
	led_data[bit_index].level1 = 0;
	led_data[bit_index].duration1 = 8;
}

esp_err_t WS2812::show() {
	return rmtWrite(rmt_send, led_data, ledCounts*24);
}
