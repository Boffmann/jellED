#include "beatdetection.h"

#include "soundconfig.h"
#include <Arduino.h>

BeatDetector::BeatDetector() :
    fft_sample_index{0},
    fft_mag_sampling_count{1},
    fft_magnitude{0},
    previous_peak{0},
    last_beat_time{millis()} {
    fft_analysis = fft_init(NUM_FFT_SAMPLES, FFT_REAL, FFT_FORWARD, input_buffer, output_buffer);
    peakDetector.begin(10, 2, 0.9);
}

bool BeatDetector::is_beat(const int sample) {
    fft_analysis->input[fft_sample_index] = (float) sample;

    if (++fft_sample_index >= NUM_FFT_SAMPLES) {
        fft_sample_index = 0;
    } else {
        return false;
    }

    unsigned long now = millis();

    fft_dc_removal(fft_analysis);
    fft_execute(fft_analysis);

    fft_magnitude = 0;
    float mean_magnitude = 0;
    for (int k = 1 ; k < fft_analysis->size / 2 ; k++) {
        float mag = sqrt(pow(fft_analysis->output[2*k],2) + pow(fft_analysis->output[2*k+1],2))/1;

        if (k < 20) {
            mean_magnitude += mag;
        }
        float freq = (float) k / ((float) NUM_FFT_SAMPLES / (float) SAMPLE_RATE);

        if (mag > fft_magnitude) {
            fft_magnitude = mag;
            fft_frequency = freq;
        }
    }

    //mean_magnitude /= fft_analysis->size;
    mean_magnitude /= 20;
    peakDetector.add(mean_magnitude);
    int peak = peakDetector.getPeak();
    bool is_peak_rising_edge = peak == 1 && previous_peak != 1;

    bool isBeat = fft_frequency < BEAT_MAX_FREQ
    && is_peak_rising_edge
    && peakDetector.isPeak(fft_magnitude);

    previous_peak = peak;

    if (isBeat) {
        last_beat_time = millis();
        return true;
    }

    return false;
}
