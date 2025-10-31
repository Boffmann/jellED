#ifndef __ADAPTIVE_NORMALIZER_H__
#define __ADAPTIVE_NORMALIZER_H__

namespace jellED {

class AdaptiveNormalizer {
private:
    int sample_rate;
    double target_rms;
    double gain_smooth;
    double currentGain;
    double rmsAccumulator;
    int sampleCount;

public:
    AdaptiveNormalizer(int sample_rate, double target_rms, double gain_smooth);
    double apply(double sample);
};

} // end namespace jellED

#endif