#include "speaker.h"

#include <Arduino.h>
#include <driver/dac.h>

namespace jellED {

Speaker::Speaker(const uint16_t sampling_rate)
    : OUTPUT_DELAY_MICRO{1000000 / sampling_rate},
    last_output_time{0},
    sample_rate{sampling_rate} {};

void Speaker::initialize() {
   dac_output_enable(DAC_CHANNEL_2);
}

void Speaker::play(const uint8_t audio) {
    if (last_output_time > 0) {
        long time_since_last_sampling = micros() - last_output_time;
        if(time_since_last_sampling < OUTPUT_DELAY_MICRO) {
            delayMicroseconds(OUTPUT_DELAY_MICRO - time_since_last_sampling);
        }
    }

    dac_output_voltage(DAC_CHANNEL_2, audio);

    last_output_time = micros();
}

} // namespace jellED