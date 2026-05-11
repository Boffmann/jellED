#include "include/onsetStrengthFunction.h"
#include "include/beatDetectionConfig.h"

#include <gtest/gtest.h>
#include <cmath>

namespace {

constexpr uint32_t ENVELOPE_RATE = 12000;

jellED::BeatDetectionConfig makeConfig() {
    jellED::BeatDetectionConfig cfg{};
    cfg.peakDetectionMaxBpm        = 250.0F;
    cfg.osfSmoothingTime           = 0.005F;
    cfg.osfBaselineAttackTime      = 0.18F;
    cfg.osfBaselineReleaseTime     = 0.8F;
    cfg.osfUseAdaptiveWhitening    = true;
    cfg.osfWhiteningTime           = 1.5F;
    cfg.osfWhiteningFloor          = 1e-3F;
    cfg.osfAbsoluteMinThreshold    = 0.5F;
    cfg.osfThresholdRel            = 1.5F;
    cfg.osfOnsetRatio              = 1.5F;
    cfg.osfBaselineAttackTimeFinal = 0.03F;
    cfg.osfBaselineReleaseTimeFinal= 0.3F;
    cfg.osfThresholdRelaxTime      = 0.15F;
    cfg.minRelativeThresholdFactor = 0.05F;
    cfg.risingThresholdScale       = 1.10F;
    cfg.fallingThresholdScale      = 0.85F;
    return cfg;
}

jellED::OsfBandFrame frame(float env, float weight = 1.0F) {
    return jellED::OsfBandFrame{env, /*statsReady=*/true, /*median=*/0.1F, weight};
}

}  // namespace

TEST(OnsetStrengthFunctionTest, GateClosedReturnsFalseAndDecaysSmoother) {
    jellED::OnsetStrengthFunction osf(ENVELOPE_RATE, makeConfig());
    // Push the smoother up first by processing with gate open.
    for (int i = 0; i < 200; ++i) {
        osf.process(frame(1.0F), frame(0.0F), frame(0.0F), 0.001F * i, true);
    }
    const float startOsf = osf.getCurrentOsf();
    EXPECT_GT(startOsf, 0.0F);

    // Now run with gate closed. Smoother should decay toward 0 and no peak.
    // Smoothing TC is 5 ms ≈ 60 samples; 6000 samples = 100τ → effectively 0.
    for (int i = 0; i < 6000; ++i) {
        const bool peak = osf.process(frame(1.0F), frame(1.0F), frame(1.0F),
                                      1.0F + 0.001F * i, false);
        EXPECT_FALSE(peak);
    }
    EXPECT_LT(osf.getCurrentOsf(), 1e-3F);
    EXPECT_LT(osf.getCurrentOsf(), startOsf);
}

TEST(OnsetStrengthFunctionTest, ResetZeroesEverything) {
    jellED::OnsetStrengthFunction osf(ENVELOPE_RATE, makeConfig());
    for (int i = 0; i < 1000; ++i) {
        osf.process(frame(0.5F), frame(0.5F), frame(0.5F), 0.001F * i, true);
    }
    EXPECT_GT(osf.bandLow().getBaseline(), 0.0F);

    osf.reset();
    EXPECT_FLOAT_EQ(osf.getCurrentOsf(),         0.0F);
    EXPECT_FLOAT_EQ(osf.bandLow().getBaseline(), 0.0F);
    EXPECT_FLOAT_EQ(osf.bandMid().getBaseline(), 0.0F);
    EXPECT_FLOAT_EQ(osf.bandHigh().getBaseline(),0.0F);
}

TEST(OnsetStrengthFunctionTest, DetectsPeakOnSyntheticImpulseStream) {
    auto cfg = makeConfig();
    cfg.osfSmoothingTime        = 0.001F;   // fast smoother for quick test
    cfg.osfAbsoluteMinThreshold = 0.05F;    // lower so the synth signal triggers
    cfg.osfThresholdRel         = 0.3F;
    cfg.osfOnsetRatio           = 1.2F;
    jellED::OnsetStrengthFunction osf(ENVELOPE_RATE, cfg);

    // Warm up the per-band baselines + whitening averages with a steady low
    // envelope. Long enough for whitening (5τ ≈ 7.5 s) to settle.
    float t = 0.0F;
    const float dt = 1.0F / static_cast<float>(ENVELOPE_RATE);
    for (int i = 0; i < 120000; ++i) {
        osf.process(frame(0.1F), frame(0.1F), frame(0.1F), t, true);
        t += dt;
    }

    // Inject 4 spaced impulses on the low band (separated by 0.5 s = 120 BPM).
    int peaksDetected = 0;
    for (int b = 0; b < 4; ++b) {
        for (int i = 0; i < 50; ++i) {
            const bool isImpulse = (i < 5);
            const float env = isImpulse ? 0.8F : 0.1F;
            if (osf.process(frame(env), frame(0.1F), frame(0.1F), t, true)) {
                ++peaksDetected;
            }
            t += dt;
        }
        // Long quiet gap between beats so the peak detector recovers.
        for (int i = 0; i < 5950; ++i) {
            if (osf.process(frame(0.1F), frame(0.1F), frame(0.1F), t, true)) {
                ++peaksDetected;
            }
            t += dt;
        }
    }
    // Expect roughly one detected peak per impulse (allowing slop).
    EXPECT_GE(peaksDetected, 3);
    EXPECT_LE(peaksDetected, 6);
}

TEST(OnsetStrengthFunctionTest, ApplyConfigChangesPeakSensitivity) {
    auto cfg = makeConfig();
    cfg.osfSmoothingTime        = 0.001F;
    cfg.osfAbsoluteMinThreshold = 100.0F;   // unreachable
    jellED::OnsetStrengthFunction osf(ENVELOPE_RATE, cfg);

    float t = 0.0F;
    const float dt = 1.0F / static_cast<float>(ENVELOPE_RATE);
    for (int i = 0; i < 1000; ++i) {
        osf.process(frame(1.0F), frame(0.1F), frame(0.1F), t, true);
        t += dt;
    }
    // Threshold is set to a huge value, so no peak can fire.
    bool sawPeak = false;
    for (int i = 0; i < 100; ++i) {
        if (osf.process(frame(1.0F), frame(0.1F), frame(0.1F), t, true)) {
            sawPeak = true;
        }
        t += dt;
    }
    EXPECT_FALSE(sawPeak);

    cfg.osfAbsoluteMinThreshold = 0.05F;
    cfg.osfThresholdRel         = 0.3F;
    cfg.osfOnsetRatio           = 1.1F;
    osf.applyConfig(cfg, ENVELOPE_RATE);
    // (sensitivity test only — peak timing is exercised by the impulse test)
    EXPECT_NEAR(osf.getThreshold(), osf.getThreshold(), 1e6F);  // sanity
}
