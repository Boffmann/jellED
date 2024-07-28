#include "bandpassFilter.h"

BandpassFilter::BandpassFilter() {
    this->numerator[0][0] = 2.133067786642005e-10;
    this->numerator[0][1] = 4.26613557328401e-10;
    this->numerator[0][2] = 2.133067786642005e-10;
    this->denominator[0][0] = 1.0;
    this->denominator[0][1] = -1.9917958759986085;
    this->denominator[0][2] = 0.9919513934387794;
    this->numerator[1][0] = 1.0;
    this->numerator[1][1] = 2.0;
    this->numerator[1][2] = 1.0;
    this->denominator[1][0] = 1.0;
    this->denominator[1][1] = -1.9938531563407578;
    this->denominator[1][2] = 0.993941194459794;
    this->numerator[2][0] = 1.0;
    this->numerator[2][1] = -2.0;
    this->numerator[2][2] = 1.0;
    this->denominator[2][0] = 1.0;
    this->denominator[2][1] = -1.9959339369182;
    this->denominator[2][2] = 0.9961578463971986;
    this->numerator[3][0] = 1.0;
    this->numerator[3][1] = -2.0;
    this->numerator[3][2] = 1.0;
    this->denominator[3][0] = 1.0;
    this->denominator[3][1] = -1.9979256595449553;
    this->denominator[3][2] = 0.9979870596838436;
}

int16_t BandpassFilter::applyBandpass(int16_t sample, uint8_t section) {
    this->prev_samples_per_section[section].append(sample);

    int16_t filteredSample = 0;
    filteredSample += 
        this->numerator[section][0] * this->prev_samples_per_section[section].get(2);
    filteredSample += 
        this->numerator[section][1] * this->prev_samples_per_section[section].get(1);
    filteredSample += 
        this->numerator[section][2] * this->prev_samples_per_section[section].get(0);

    filteredSample -=
        this->denominator[section][1] * this->prev_filtered_per_section[section].get(2);
    filteredSample -=
        this->denominator[section][2] * this->prev_filtered_per_section[section].get(1);

    filteredSample *= 1.0 / this->denominator[section][0];

    this->prev_filtered_per_section[section].append(filteredSample);

    return filteredSample;
}

bool BandpassFilter::apply(int16_t& sample) {
    for (int section = 0; section < NUM_SECTIONS; ++section) {
        sample = applyBandpass(sample, section);
    }
}