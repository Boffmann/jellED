#ifndef __AUTOMATIC_GAIN_CONTROL_JELLED_H__
#define __AUTOMATIC_GAIN_CONTROL_JELLED_H__

#include <stdint.h>

namespace jellED {

    class AutomaticGainControl {
        private:
            float peak_limiter_gain;
            float peak_envelope;

            float section_gain;
            float section_rms;
            uint32_t section_counter;

            float local_peak;

            uint32_t sample_rate;
            float target_level;

            // Coefficients
            float peak_attack_coeff;
            float peak_release_coeff;
            float section_alpha;
            float local_alpha;

        public:
            AutomaticGainControl(uint32_t sample_rate, float target_level);

            float apply(float sample);

            float getSectionGain() const;
            float getPeakLimiterGain() const;
            float getCurrentRMS() const;
        };

} // end namespace jellED

#endif
