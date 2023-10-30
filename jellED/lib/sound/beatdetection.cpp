#include "beatdetection.h"

#include "soundconfig.h"
#include <arduinoFFT.h>

// https://www.hackster.io/mamifero/arduino-beat-detector-d0a21f

// https://github.com/blaz-r/ESP32-music-beat-sync/blob/main/src/ESP32-music-beat-sync.cpp

BeatDetector::BeatDetector() :
    fft_sample_index{0},
    fft_mag_sampling_count{1},
    last_beat_time{millis()} {
    FFT = arduinoFFT(vReal, vImag, NUM_FFT_SAMPLES, (double) SAMPLE_RATE);
}

bool BeatDetector::is_beat_possible() {
    return millis() - last_beat_time > BEAT_SPAN_MILLIS;
}

bool BeatDetector::is_beat(const int sample) {
    vReal[fft_sample_index] = (float) sample;
    vImag[fft_sample_index] = 0.0f;

    if (++fft_sample_index >= NUM_FFT_SAMPLES) {
        fft_sample_index = 0;
    } else {
        return false;
    }

    if (!is_beat_possible()) {
        return false;
    }

    this->FFT.DCRemoval();
    this->FFT.Compute(FFTDirection::Forward);
    this->FFT.ComplexToMagnitude();
    this->FFT.MajorPeak(&fft_frequency, &fft_magnitude);

    fft_magnitude = 0;
    for (int b = 0; b < NUM_FFT_SAMPLES / 8; b++) {
        fft_magnitude += vReal[b];
    }
    fft_magnitude /= (NUM_FFT_SAMPLES / 8);

    // adjust running average
    fft_avg_magnitude = fft_avg_magnitude + (fft_magnitude - fft_avg_magnitude) / fft_mag_sampling_count;
    fft_mag_sampling_count++;

    // when sample count reaches limit, reset to lower value, this way we avoid slow change on music change
    if (fft_mag_sampling_count > AVG_COUNT_LIMIT) {
        fft_mag_sampling_count = AVG_COUNT_LOWER;
    }

    bool isBeat = is_beat_possible()
    && fft_frequency < BEAT_MAX_FREQ
    && fft_magnitude > fft_avg_magnitude * 1.2;

    if (isBeat) {
        last_beat_time = millis();
        return true;
    }

    return false;
}
