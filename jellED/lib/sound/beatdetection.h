#ifndef _MUSIC_PIECE_JELLED_TEST_H_
#define _MUSIC_PIECE_JELLED_TEST_H_

#include <stdint.h>
#include <arduinoFFT.h>

class BeatDetector {
private:
    static const uint16_t NUM_FFT_SAMPLES = 1024;
    /*Max Frequency that still counts as a beat*/
    static const uint16_t BEAT_MAX_FREQ = 150;
    static const uint16_t AVG_COUNT_LIMIT  = 500;
    static const uint8_t AVG_COUNT_LOWER  = 100;
    static const uint8_t MAX_BPM = 180;
    static constexpr double BEAT_SPAN_MILLIS = 60.0 / MAX_BPM * 1000;
    uint16_t fft_sample_index, fft_mag_sampling_count;

    double vReal[NUM_FFT_SAMPLES];
    double vImag[NUM_FFT_SAMPLES];
    double fft_frequency, fft_magnitude, fft_avg_magnitude;
    unsigned long last_beat_time;

    arduinoFFT FFT;
    bool is_beat_possible();

public:
    BeatDetector();
    bool is_beat(const int sample);
};

#endif