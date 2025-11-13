#include "include/bandpassFilter.h"

#include <iostream>

namespace jellED {

BandpassFilter::BandpassFilter()
: prev_samples_per_section{nullptr},
  prev_filtered_per_section{nullptr}
 {
    this->prev_samples_per_section = (Ringbuffer**) malloc(sizeof(Ringbuffer*) * NUM_SECTIONS);
    this->prev_filtered_per_section = (Ringbuffer**) malloc(sizeof(Ringbuffer*) * NUM_SECTIONS);

    for (int ring_buffer_index = 0; ring_buffer_index < NUM_SECTIONS; ring_buffer_index++) {
        this->prev_samples_per_section[ring_buffer_index] = new Ringbuffer(3);
        this->prev_filtered_per_section[ring_buffer_index] = new Ringbuffer(3);
    }
    // Butterworth coefficients: 50 Hz - 200 Hz, order: 4, framerate: 12000
    this->numerator[0][0] = 2.1505687372880117e-06;
    this->numerator[0][1] = 4.3011374745760235e-06;
    this->numerator[0][2] = 2.1505687372880117e-06;
    this->denominator[0][0] = 1.0;
    this->denominator[0][1] = -1.900968573046465;
    this->denominator[0][2] = 0.9064322445782939;
    this->numerator[1][0] = 1.0;
    this->numerator[1][1] = 2.0;
    this->numerator[1][2] = 1.0;
    this->denominator[1][0] = 1.0;
    this->denominator[1][1] = -1.943579632456509;
    this->denominator[1][2] = 0.9536573669275572;
    this->numerator[2][0] = 1.0;
    this->numerator[2][1] = -2.0;
    this->numerator[2][2] = 1.0;
    this->denominator[2][0] = 1.0;
    this->denominator[2][1] = -1.9528162428273361;
    this->denominator[2][2] = 0.9540976768424779;
    this->numerator[3][0] = 1.0;
    this->numerator[3][1] = -2.0;
    this->numerator[3][2] = 1.0;
    this->denominator[3][0] = 1.0;
    this->denominator[3][1] = -1.98673867006111;
    this->denominator[3][2] = 0.9874620427225849;
}

BandpassFilter::~BandpassFilter() {
    if (this->prev_samples_per_section != nullptr) {
        for (int ring_buffer_index = 0; ring_buffer_index < NUM_SECTIONS; ring_buffer_index++) {
            delete this->prev_samples_per_section[ring_buffer_index];
        }
        free(prev_samples_per_section);
    }
    if (this->prev_filtered_per_section != nullptr) {
        for (int ring_buffer_index = 0; ring_buffer_index < NUM_SECTIONS; ring_buffer_index++) {
            delete this->prev_filtered_per_section[ring_buffer_index];
        }
        free(prev_filtered_per_section);
    }
}

double BandpassFilter::applyBandpass(double sample, uint8_t section) {
    this->prev_samples_per_section[section]->append(sample);

    double filteredSample = 0.0;
    filteredSample +=
        this->numerator[section][0] * this->prev_samples_per_section[section]->get(2);
    filteredSample +=
        this->numerator[section][1] * this->prev_samples_per_section[section]->get(1);
    filteredSample +=
        this->numerator[section][2] * this->prev_samples_per_section[section]->get(0);

    filteredSample -=
        this->denominator[section][1] * this->prev_filtered_per_section[section]->get(2);
    filteredSample -=
        this->denominator[section][2] * this->prev_filtered_per_section[section]->get(1);

    filteredSample *= 1.0 / this->denominator[section][0];

    this->prev_filtered_per_section[section]->append(filteredSample);

    return filteredSample;
}

double BandpassFilter::apply(const double sample) {
    double filtered_sample = sample;
    for (int section = 0; section < NUM_SECTIONS; ++section) {
        filtered_sample = applyBandpass(filtered_sample, section);
    }

    return filtered_sample;
}

} // end namespace jellED
