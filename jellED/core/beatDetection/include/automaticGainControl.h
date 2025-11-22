#ifndef __AUTOMATIC_GAIN_CONTROL_JELLED_H__
#define __AUTOMATIC_GAIN_CONTROL_JELLED_H__

#include <stdint.h>

namespace jellED {

    class AutomaticGainControl {
        private:
            double peak_limiter_gain;
            double peak_envelope;
            
            double section_gain;
            double section_rms;
            uint32_t section_counter;
            
            double local_peak;
            
            uint32_t sample_rate;
            double target_level;
            
            // Coefficients
            double peak_attack_coeff;
            double peak_release_coeff;
            double section_alpha;
            double local_alpha;
            
        public:
            AutomaticGainControl(uint32_t sample_rate, double target_level);
            
            double apply(double sample);
            
            double getSectionGain() const;
            double getPeakLimiterGain() const;
            double getCurrentRMS() const;
        };

} // end namespace jellED

#endif
