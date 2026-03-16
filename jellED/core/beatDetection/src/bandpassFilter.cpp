#include "include/bandpassFilter.h"

#include <iostream>

namespace jellED {

BandpassFilter::BandpassFilter(const BandpassFilterCoefficients& coefficients)
: coefficients(coefficients),
  prev_samples_per_section{nullptr},
  prev_filtered_per_section{nullptr}
{
    this->prev_samples_per_section = (Ringbuffer**) malloc(sizeof(Ringbuffer*) * NUM_SECTIONS);
    this->prev_filtered_per_section = (Ringbuffer**) malloc(sizeof(Ringbuffer*) * NUM_SECTIONS);

    for (int ring_buffer_index = 0; ring_buffer_index < NUM_SECTIONS; ring_buffer_index++) {
        this->prev_samples_per_section[ring_buffer_index] = new Ringbuffer(3);
        this->prev_filtered_per_section[ring_buffer_index] = new Ringbuffer(3);
    }
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

float BandpassFilter::applyBandpass(float sample, uint8_t section) {
    this->prev_samples_per_section[section]->append(sample);

    float filteredSample = 0.0f;
    filteredSample +=
        this->coefficients.numerator[section][0] * this->prev_samples_per_section[section]->get(2);
    filteredSample +=
        this->coefficients.numerator[section][1] * this->prev_samples_per_section[section]->get(1);
    filteredSample +=
        this->coefficients.numerator[section][2] * this->prev_samples_per_section[section]->get(0);

    filteredSample -=
        this->coefficients.denominator[section][1] * this->prev_filtered_per_section[section]->get(2);
    filteredSample -=
        this->coefficients.denominator[section][2] * this->prev_filtered_per_section[section]->get(1);

    this->prev_filtered_per_section[section]->append(filteredSample);

    return filteredSample;
}

float BandpassFilter::apply(const float sample) {
    float filtered_sample = sample;
    for (int section = 0; section < NUM_SECTIONS; ++section) {
        filtered_sample = applyBandpass(filtered_sample, section);
    }

    return filtered_sample;
}

} // end namespace jellED
