#include "beatdetection.h"

#include "soundconfig.h"
#include <Arduino.h>

// https://www.hackster.io/mamifero/arduino-beat-detector-d0a21f

// https://github.com/blaz-r/ESP32-music-beat-sync/blob/main/src/ESP32-music-beat-sync.cpp

// https://github.com/yash-sanghvi/ESP32/blob/master/FFT_on_ESP32_Arduino/FFT_on_ESP32_Arduino.ino

BeatDetector::BeatDetector() :
    fft_sample_index{0},
    fft_mag_sampling_count{1},
    fft_magnitude{0},
    last_beat_time{millis()} {
    //FFT = arduinoFFT(vReal, vImag, NUM_FFT_SAMPLES, (double) SAMPLE_RATE);
    fft_analysis = fft_init(NUM_FFT_SAMPLES, FFT_REAL, FFT_FORWARD, input_buffer, output_buffer);
}

bool BeatDetector::is_beat_possible() {
    return millis() - last_beat_time > BEAT_SPAN_MILLIS;
}

bool BeatDetector::is_beat(const int sample) {
    //vReal[fft_sample_index] = (float) sample;
    //vImag[fft_sample_index] = 0.0f;
    //input_buffer[fft_sample_index] = (float) sample;
    //input_buffer[fft_sample_index + 1] = 0.0f;

    fft_analysis->input[fft_sample_index] = (float) sample;

    if (++fft_sample_index >= NUM_FFT_SAMPLES) {
        fft_sample_index = 0;
    } else {
        return false;
    }

    if (!is_beat_possible()) {
        return false;
    }

    unsigned long now = millis();

    fft_dc_removal(fft_analysis);
    fft_execute(fft_analysis);

    fft_magnitude = 0;
    float mean_magnitude = 0;
    //float mean_max_mag = 0;
    for (int k = 1 ; k < fft_analysis->size / 2 ; k++) {
        float mag = sqrt(pow(fft_analysis->output[2*k],2) + pow(fft_analysis->output[2*k+1],2))/1;
        mean_magnitude += mag;
        float freq = (float) k / ((float) NUM_FFT_SAMPLES /  (float) SAMPLE_RATE);

        if (mag > fft_magnitude) {
            fft_magnitude = mag;
            fft_frequency = freq;
        }
    }
    mean_magnitude /= fft_analysis->size;
    //mean_max_mag += fft_magnitude;
    //mean_max_mag /= fft_mag_sampling_count;

    // adjust running average
    fft_avg_magnitude = fft_avg_magnitude + (mean_magnitude - fft_avg_magnitude) / fft_mag_sampling_count;
    fft_mag_sampling_count++;

    // when sample count reaches limit, reset to lower value, this way we avoid slow change on music change
    if (fft_mag_sampling_count > AVG_COUNT_LIMIT) {
        fft_mag_sampling_count = AVG_COUNT_LOWER;
    }
    

    bool isBeat = fft_frequency < BEAT_MAX_FREQ
    && mean_magnitude > fft_avg_magnitude * 1.2;

    //float dc_component = fft_analysis->output[0]/NUM_FFT_SAMPLES;
    //Serial.print("FFT dc component: ");
    //Serial.println(dc_component);

    //Serial.print("FFT Magnitude: ");
    //Serial.println(fft_magnitude);

    //Serial.print("FFT Avg Magnitude: ");
    //Serial.println(fft_avg_magnitude);

    //Serial.print("FFT Frequency: ");
    //Serial.println(fft_frequency);

    //Serial.print("FFT Processing Time: ");
    //Serial.println(millis() - now);

    if (isBeat) {
        last_beat_time = millis();
        return true;
    }

    return false;
}
