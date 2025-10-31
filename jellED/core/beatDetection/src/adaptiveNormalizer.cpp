#include "include/adaptiveNormalizer.h"

#include <algorithm>
#include <cmath>

namespace jellED {

AdaptiveNormalizer::AdaptiveNormalizer(int sample_rate, double target_rms, double gain_smooth)
    : sample_rate(sample_rate),
      target_rms(target_rms),
      gain_smooth(gain_smooth),
      currentGain(1.0),
      rmsAccumulator(0.0),
      sampleCount(0)
{
}

double AdaptiveNormalizer::apply(double sample) {
    // Accumulate RMS calculation
    rmsAccumulator += sample * sample;
    sampleCount++;
    
    // Update gain every ANALYSIS_WINDOW samples (slowly)
    if (sampleCount >= this->sample_rate) {
        double rms = std::sqrt(rmsAccumulator / sampleCount);
        
        // Only adjust if signal is significantly off-target
        if (rms > 0.01) {  // Avoid amplifying noise
            double targetGain = this->target_rms / rms;
            
            // Limit gain range to avoid extremes
            targetGain = std::max(1.0, std::min(10.0, targetGain));
            
            // Very smooth transition to new gain
            currentGain = this->gain_smooth * currentGain + (1.0 - this->gain_smooth) * targetGain;
        }
        
        // Reset accumulator
        rmsAccumulator = 0.0;
        sampleCount = 0;
    }
    
    return sample * currentGain;
}

} // end namespace jellED