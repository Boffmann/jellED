#ifndef _BEAT_DETECTION_CONFIG_H_
#define _BEAT_DETECTION_CONFIG_H_

namespace jellED {

struct BeatDetectionConfig {
    int envelopeDownsampleRatio = 1;
    double downsampleCutoffFrequency = 0.5;
    double automaticGainControlTargetLevel = 0.4;
    double absoluteMinThresholdLow = 0.05;
    double absoluteMinThresholdMid = 0.05;
    double absoluteMinThresholdHigh = 0.05;
    double thresholdRelLow = 0.15;
    double thresholdRelMid = 0.15;
    double thresholdRelHigh = 0.15;
    double peakDetectionMaxBpm = 250.0;
    double bandWeightLow = 1.0;
    double bandWeightMid = 0.5;
    double bandWeightHigh = 0.3;
    // How fast the envelope detector responds to rising energy (seconds).
    // Shorter = catches sharp transients. Hi-hats need very fast attack;
    // bass can tolerate a slightly longer attack which smooths out
    // sub-harmonic noise spikes.
    double envelopeAttackTimeLow  = 0.005;
    double envelopeAttackTimeMid  = 0.005;
    double envelopeAttackTimeHigh = 0.005;

    // How fast the envelope detector decays after a transient (seconds).
    // Bass energy rings longer, so a slower release (larger value) produces a
    // smoother envelope and avoids double-triggers. A shorter release for the
    // high band lets the envelope drop between rapid hi-hat hits.
    double envelopeReleaseTimeLow  = 0.050;
    double envelopeReleaseTimeMid  = 0.050;
    double envelopeReleaseTimeHigh = 0.050;

    double coincidenceWindow = 0.15;

    // --- Peak detector timing (per-band) ---

    // How fast the peak detector's internal baseline tracks rising energy.
    // Shorter = more responsive to transients. Bass instruments have slower
    // attacks than hi-hats, so the low band can use a larger value.
    double baselineAttackTimeLow  = 0.03;
    double baselineAttackTimeMid  = 0.03;
    double baselineAttackTimeHigh = 0.03;

    // How fast the peak detector's internal baseline drops after loud passages.
    // Bass energy sustains longer than high-frequency energy, so the low band
    // benefits from a slower release (larger value).
    double baselineReleaseTimeLow  = 0.3;
    double baselineReleaseTimeMid  = 0.3;
    double baselineReleaseTimeHigh = 0.3;

    // After a peak is detected the dynamic threshold is boosted and then
    // exponentially relaxes back to the floor over this time. Controls the
    // refractory period between consecutive peaks. A shorter time lets rapid
    // successive peaks through (useful for hi-hats), while a longer time
    // prevents double-triggers (useful for kicks).
    double thresholdRelaxTimeLow  = 0.3;
    double thresholdRelaxTimeMid  = 0.3;
    double thresholdRelaxTimeHigh = 0.3;

    // Required ratio between the current peak amplitude and the recent valley
    // minimum for a peak to be accepted. Prevents sustained noise from
    // triggering false peaks. High-frequency energy (cymbals) is more sustained,
    // so a lower ratio helps avoid false negatives. Bass peaks are more
    // prominent, so a higher ratio helps avoid false positives.
    double onsetRatioLow  = 1.2;
    double onsetRatioMid  = 1.2;
    double onsetRatioHigh = 1.2;

    // --- Peak detector tuning (global) ---

    // Floor for the dynamic threshold, expressed as a fraction of thresholdRel.
    // After the refractory period the threshold decays down to
    // thresholdRel * minRelativeThresholdFactor. Lower = more sensitive to
    // quiet peaks after loud ones.
    double minRelativeThresholdFactor = 0.05;

    // Schmitt-trigger hysteresis: the envelope must exceed
    // threshold * risingThresholdScale to begin considering a peak, and must
    // drop below threshold * fallingThresholdScale to reset. Wider gap = more
    // noise immunity, narrower = more responsive.
    double risingThresholdScale  = 1.10;
    double fallingThresholdScale = 0.85;
};

} // namespace jellED

#endif
