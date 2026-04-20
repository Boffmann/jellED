#include <gtest/gtest.h>

#include <cmath>

#include "expFilter.h"

using jellED::ExpFilter;

namespace {

constexpr float kTolerance = 1e-4f;

} // namespace

TEST(ExpFilterTest, InitialValueIsPreserved) {
    ExpFilter<float> f(0.42f, 0.1f, 0.1f);
    EXPECT_FLOAT_EQ(f.value(), 0.42f);
}

TEST(ExpFilterTest, ZeroDtLeavesValueUnchanged) {
    ExpFilter<float> f(1.0f, 0.05f, 0.5f);
    const float out = f.update(10.0f, 0.0f);
    // alpha = 1 - exp(0) = 0, so output = old value.
    EXPECT_FLOAT_EQ(out, 1.0f);
    EXPECT_FLOAT_EQ(f.value(), 1.0f);
}

TEST(ExpFilterTest, LargeDtSnapsToInput) {
    ExpFilter<float> f(0.0f, 0.01f, 0.01f);
    // dt = 1s with tau = 10ms → alpha ≈ 1 - exp(-100) ≈ 1.
    const float out = f.update(5.0f, 1.0f);
    EXPECT_NEAR(out, 5.0f, kTolerance);
}

TEST(ExpFilterTest, SymmetricTauBehavesAsOrdinaryLowpass) {
    // Step input from 0 to 1. After one time constant the output should
    // reach ~63% (1 - 1/e) of the step.
    const float tau = 0.05f;
    ExpFilter<float> f(0.0f, tau, tau);
    const float out = f.update(1.0f, tau);
    EXPECT_NEAR(out, 1.0f - 1.0f / std::exp(1.0f), kTolerance);
}

TEST(ExpFilterTest, PeakFollowerSnapsUpSlowlyDecays) {
    // Fast rise, slow decay — classic envelope detector.
    ExpFilter<float> f(0.0f, /*tau_rise*/ 0.001f, /*tau_decay*/ 1.0f);

    // One update with big input → should snap up very close to input.
    f.update(1.0f, 0.01f);
    EXPECT_GT(f.value(), 0.99f);

    // Now input drops to 0. Output should decay slowly.
    const float before = f.value();
    f.update(0.0f, 0.01f);
    // After 10ms with tau_decay=1s: alpha ≈ 0.01, so output loses only ~1%.
    EXPECT_NEAR(f.value(), before * (1.0f - 0.01f), 1e-3f);
}

TEST(ExpFilterTest, BaselineTrackerSnapsDownSlowlyRises) {
    // Slow rise, fast decay — baseline/common-mode tracker.
    ExpFilter<float> f(1.0f, /*tau_rise*/ 1.0f, /*tau_decay*/ 0.001f);

    // Input drops to 0 → output should snap down fast.
    f.update(0.0f, 0.01f);
    EXPECT_LT(f.value(), 0.01f);

    // Input rises to 1 → output should rise slowly.
    f.update(1.0f, 0.01f);
    // After 10ms with tau_rise=1s, alpha ≈ 0.01.
    EXPECT_LT(f.value(), 0.02f);
}

TEST(ExpFilterTest, ResetOverwritesState) {
    ExpFilter<float> f(1.0f, 0.1f, 0.1f);
    f.update(0.5f, 0.05f);
    ASSERT_NE(f.value(), 42.0f);
    f.reset(42.0f);
    EXPECT_FLOAT_EQ(f.value(), 42.0f);
}

TEST(ExpFilterTest, SetTimeConstantsAffectsSubsequentUpdates) {
    ExpFilter<float> f(0.0f, 10.0f, 10.0f); // very slow initially
    f.update(1.0f, 0.01f);
    const float slow_result = f.value();
    EXPECT_LT(slow_result, 0.01f); // barely moved

    // Now make it very fast and repeat the same update from the same state.
    f.reset(0.0f);
    f.setTimeConstants(0.001f, 0.001f);
    f.update(1.0f, 0.01f);
    const float fast_result = f.value();
    EXPECT_GT(fast_result, 0.99f);
}

TEST(ExpFilterTest, TimeAwarenessIsConsistentAcrossCallRates) {
    // The core value proposition of time-awareness: reaching the same state
    // after the same total elapsed time, regardless of update granularity.
    const float tau = 0.1f;
    const float input = 1.0f;
    const float total_time = 0.2f;

    // One big step.
    ExpFilter<float> coarse(0.0f, tau, tau);
    coarse.update(input, total_time);

    // Twenty small steps.
    ExpFilter<float> fine(0.0f, tau, tau);
    const int steps = 20;
    const float dt = total_time / steps;
    for (int i = 0; i < steps; ++i) {
        fine.update(input, dt);
    }

    // Should converge to roughly the same value (not identical — the IIR
    // recurrence is an approximation to the continuous system).
    EXPECT_NEAR(coarse.value(), fine.value(), 1e-2f);
}
