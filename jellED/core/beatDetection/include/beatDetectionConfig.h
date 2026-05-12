#ifndef _BEAT_DETECTION_CONFIG_H_
#define _BEAT_DETECTION_CONFIG_H_

namespace jellED {

enum class ThresholdMode { Envelope, MovingMean };

struct BeatDetectionConfig {
    int envelopeDownsampleRatio = 1;
    float downsampleCutoffFrequency = 0.5;
    float automaticGainControlTargetLevel = 0.4;
    float noiseGateThreshold = 0.005;
    float absoluteMinThresholdLow = 0.05;
    float absoluteMinThresholdMid = 0.05;
    float absoluteMinThresholdHigh = 0.05;
    float thresholdRelLow = 0.15;
    float thresholdRelMid = 0.15;
    float thresholdRelHigh = 0.15;
    float peakDetectionMaxBpm = 250.0;
    float bandWeightLow = 1.0;
    float bandWeightMid = 0.5;
    float bandWeightHigh = 0.3;
    // How fast the envelope detector responds to rising energy (seconds).
    // Shorter = catches sharp transients. Hi-hats need very fast attack;
    // bass can tolerate a slightly longer attack which smooths out
    // sub-harmonic noise spikes.
    float envelopeAttackTimeLow  = 0.005;
    float envelopeAttackTimeMid  = 0.005;
    float envelopeAttackTimeHigh = 0.005;

    // How fast the envelope detector decays after a transient (seconds).
    // Bass energy rings longer, so a slower release (larger value) produces a
    // smoother envelope and avoids float-triggers. A shorter release for the
    // high band lets the envelope drop between rapid hi-hat hits.
    float envelopeReleaseTimeLow  = 0.050;
    float envelopeReleaseTimeMid  = 0.050;
    float envelopeReleaseTimeHigh = 0.050;

    float coincidenceWindow = 0.15;

    // --- Peak detector timing (per-band) ---

    // How fast the peak detector's internal baseline tracks rising energy.
    // Shorter = more responsive to transients. Bass instruments have slower
    // attacks than hi-hats, so the low band can use a larger value.
    float baselineAttackTimeLow  = 0.03;
    float baselineAttackTimeMid  = 0.03;
    float baselineAttackTimeHigh = 0.03;

    // How fast the peak detector's internal baseline drops after loud passages.
    // Bass energy sustains longer than high-frequency energy, so the low band
    // benefits from a slower release (larger value).
    float baselineReleaseTimeLow  = 0.3;
    float baselineReleaseTimeMid  = 0.3;
    float baselineReleaseTimeHigh = 0.3;

    // After a peak is detected the dynamic threshold is boosted and then
    // exponentially relaxes back to the floor over this time. Controls the
    // refractory period between consecutive peaks. A shorter time lets rapid
    // successive peaks through (useful for hi-hats), while a longer time
    // prevents float-triggers (useful for kicks).
    float thresholdRelaxTimeLow  = 0.3;
    float thresholdRelaxTimeMid  = 0.3;
    float thresholdRelaxTimeHigh = 0.3;

    // Required ratio between the current peak amplitude and the recent valley
    // minimum for a peak to be accepted. Prevents sustained noise from
    // triggering false peaks. High-frequency energy (cymbals) is more sustained,
    // so a lower ratio helps avoid false negatives. Bass peaks are more
    // prominent, so a higher ratio helps avoid false positives.
    float onsetRatioLow  = 1.2;
    float onsetRatioMid  = 1.2;
    float onsetRatioHigh = 1.2;

    // --- Peak detector tuning (global) ---

    // Floor for the dynamic threshold, expressed as a fraction of thresholdRel.
    // After the refractory period the threshold decays down to
    // thresholdRel * minRelativeThresholdFactor. Lower = more sensitive to
    // quiet peaks after loud ones.
    float minRelativeThresholdFactor = 0.05;

    // Schmitt-trigger hysteresis: the envelope must exceed
    // threshold * risingThresholdScale to begin considering a peak, and must
    // drop below threshold * fallingThresholdScale to reset. Wider gap = more
    // noise immunity, narrower = more responsive.
    float risingThresholdScale  = 1.10;
    float fallingThresholdScale = 0.85;

    // --- Tempo-locked acceptance ---
    // When enabled, accepted peaks are gated against the tempo grid maintained
    // by TempoTracker: candidate peaks whose interval-since-last-accepted-beat
    // doesn't fit the established tempo are rejected. Off-grid noise peaks
    // (crowd cheers, rumble, etc.) are filtered out without adding latency to
    // accepted beats. Disabled by default so existing behavior is preserved
    // until enabled via the configurator.
    bool useTempoLock = false;

    // Absolute BPM bounds the tempo tracker accepts. Anything outside is
    // rejected even before tempo establishment. Tightened from the
    // TempoTracker default (70-200) to match the festival/techno context.
    float tempoLockMinBpm = 90.0;
    float tempoLockMaxBpm = 180.0;

    // Fractional tolerance around the established IBI median. Smaller =
    // tighter grid lock = more rejection. Larger = easier re-acquisition
    // after tempo changes.
    float tempoLockTolerance = 0.20;

    // After this many seconds with no accepted beats, the tempo history is
    // cleared so the gate cold-starts cleanly on the next track.
    float tempoLockStaleResetTime = 4.0;

    // --- OSF (Onset Strength Function) fusion ---
    // When true, is_beat() returns peaks from a single peak detector run on the
    // weighted sum of per-band normalized novelties (Bello et al. 2005). When
    // false, falls back to legacy low-band-only behavior. Default-off so the
    // production path is unchanged until A/B tested on festival audio.
    bool useOsfFusion = false;

    // Per-band novelty baseline. Slow on purpose: tracks the recent *floor*
    // rather than the recent envelope, so it does not climb into onsets and
    // cancel them. A faster attack noticeably eats sustained snare rolls.
    float osfBaselineAttackTime  = 0.18;
    float osfBaselineReleaseTime = 0.8;

    // Adaptive whitening (Stowell & Plumbley 2007). A slow per-band EMA of the
    // envelope acts as the divisor for novelty normalization; survives sub-bass
    // nulls (mic in standing-wave shadow) that median normalization cannot.
    bool  osfUseAdaptiveWhitening = true;
    float osfWhiteningTime        = 1.5;
    float osfWhiteningFloor       = 1e-3;

    // OSF post-sum smoothing (one-pole LPF on the summed signal). Runs at the
    // envelope rate, not the input rate.
    float osfSmoothingTime = 0.005;

    // OSF peak detector (separate parameter set from per-band detectors). The
    // OSF is dimensionless after whitening so absolute thresholds are unitless
    // multipliers, not signal-level floors.
    float osfAbsoluteMinThreshold     = 0.5;
    float osfThresholdRel             = 1.5;
    float osfOnsetRatio               = 1.5;
    float osfBaselineAttackTimeFinal  = 0.03;
    float osfBaselineReleaseTimeFinal = 0.3;
    float osfThresholdRelaxTime       = 0.15;

    // Spectral-tilt-aware band weighting. Uses the existing getSpectralTilt()
    // signal to bias weights toward low when bass-heavy and high when
    // treble-heavy — addresses build-up/drop adaptation when the kick drops
    // out for 8-32 bars. Off by default so the algorithm change can be
    // evaluated independently of the tilt biasing.
    bool  osfSpectralTiltWeighting = false;
    float osfTiltGain              = 0.3;

    // Suppress OSF beats when the short-term energy is below this floor.
    // Rejects between-track silence and crowd-only periods without needing a
    // separate state machine.
    float osfOverallLevelGate = 0.01;

    // --- Moving-mean threshold (Böck/Schedl ISMIR 2012) ---
    // When thresholdMode == MovingMean, PeakDetector replaces the asymmetric
    // envelope follower with a causal moving-mean window. Threshold becomes:
    //   max(absoluteMinThreshold, mean(window) + thresholdDelta)
    // The window recovers symmetrically within W frames vs. ~τ_release for the
    // envelope follower, eliminating post-peak masking of quieter subsequent beats.
    ThresholdMode thresholdMode = ThresholdMode::Envelope;

    // Per-band window size in milliseconds. Longer windows track slower-evolving
    // dynamics: bass has more sustained energy (300ms), highs are more transient
    // (150ms). These are the Böck 2012 "pre_avg" parameter scaled to milliseconds.
    float thresholdWindowMsLow  = 300.0;
    float thresholdWindowMsMid  = 200.0;
    float thresholdWindowMsHigh = 150.0;

    // Additive offset δ above the local mean (Böck 2012 "delta" parameter).
    // Acts as the minimum detection margin; use absolute signal-level units
    // (same scale as the envelope values entering the peak detector).
    float thresholdDelta = 0.05;

    // OSF-domain moving mean. The OSF signal is dimensionless after whitening
    // (typical range 0–5), so osfThresholdDelta should match osfAbsoluteMinThreshold.
    float osfThresholdWindowMs = 200.0;
    float osfThresholdDelta    = 0.5;
};

} // namespace jellED

#endif
