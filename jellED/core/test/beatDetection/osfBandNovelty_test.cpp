#include "include/onsetStrengthFunction.h"

#include <gtest/gtest.h>
#include <cmath>

namespace {

constexpr uint32_t ENVELOPE_RATE = 12000;

jellED::OsfBandNovelty makeBand() {
    jellED::OsfBandNovelty b;
    b.setBaselineTimings(0.18F, 0.8F, ENVELOPE_RATE);
    b.setWhiteningTime(1.5F, ENVELOPE_RATE);
    return b;
}

}  // namespace

TEST(OsfBandNoveltyTest, ReturnsZeroBeforeStatsReady) {
    auto band = makeBand();
    // statsReady=false → must return 0 regardless of input magnitude.
    EXPECT_FLOAT_EQ(band.compute(1.0F, false, 0.1F, true, 1e-3F), 0.0F);
    EXPECT_FLOAT_EQ(band.compute(5.0F, false, 0.1F, true, 1e-3F), 0.0F);
}

TEST(OsfBandNoveltyTest, ReturnsZeroWhenSampleAtOrBelowBaseline) {
    auto band = makeBand();
    // Settle baseline at 0.5. 5τ ≈ 0.9 s → 10800 samples; 22000 for headroom.
    for (int i = 0; i < 22000; ++i) {
        band.compute(0.5F, true, 0.1F, true, 1e-3F);
    }
    EXPECT_NEAR(band.compute(0.5F, true, 0.1F, true, 1e-3F), 0.0F, 0.01F);
    // Below baseline → HWR clamps to 0.
    EXPECT_FLOAT_EQ(band.compute(0.3F, true, 0.1F, true, 1e-3F), 0.0F);
}

TEST(OsfBandNoveltyTest, NoveltyScalesWithDifferenceOverWhiteningAvg) {
    auto band = makeBand();
    // Whitening TC is 1.5 s → 5τ ≈ 7.5 s ≈ 90000 samples; 120000 for headroom.
    for (int i = 0; i < 120000; ++i) {
        band.compute(0.2F, true, 0.1F, true, 1e-3F);
    }
    ASSERT_NEAR(band.getBaseline(),     0.2F, 0.01F);
    ASSERT_NEAR(band.getWhiteningAvg(), 0.2F, 0.01F);

    const float novelty = band.compute(0.6F, true, 0.1F, true, 1e-3F);
    // diff ≈ 0.4, divisor ≈ 0.2 → novelty ≈ 2.0.
    EXPECT_NEAR(novelty, 2.0F, 0.2F);
}

TEST(OsfBandNoveltyTest, AsymmetricBaselineAttackVsRelease) {
    auto band = makeBand();
    for (int i = 0; i < 20000; ++i) {
        band.compute(0.1F, true, 0.1F, true, 1e-3F);
    }
    const float baselineBefore = band.getBaseline();

    // Sustained step up to 0.5 for 100 ms — attack TC is 180 ms, so the
    // baseline should NOT have caught up yet.
    for (int i = 0; i < static_cast<int>(ENVELOPE_RATE * 0.1F); ++i) {
        band.compute(0.5F, true, 0.1F, true, 1e-3F);
    }
    const float baselineAfter = band.getBaseline();

    EXPECT_GT(baselineAfter, baselineBefore);
    EXPECT_LT(baselineAfter, 0.4F);  // well below the step
}

TEST(OsfBandNoveltyTest, MedianFallbackWhenWhiteningDisabled) {
    auto band = makeBand();
    // Settle baseline low so we have headroom for a positive diff.
    for (int i = 0; i < 22000; ++i) {
        band.compute(0.05F, true, 0.2F, /*useWhitening=*/false, 1e-3F);
    }
    // Spike well above baseline. Divisor = max(median=0.2, floor=1e-3) = 0.2.
    const float novelty = band.compute(0.6F, true, 0.2F, false, 1e-3F);
    // diff ≈ 0.55, divisor = 0.2 → novelty ≈ 2.75.
    EXPECT_GT(novelty, 1.5F);
    EXPECT_LT(novelty, 4.0F);
}

TEST(OsfBandNoveltyTest, ResetClearsBaselineAndWhitening) {
    auto band = makeBand();
    for (int i = 0; i < 20000; ++i) {
        band.compute(0.5F, true, 0.1F, true, 1e-3F);
    }
    EXPECT_GT(band.getBaseline(),     0.1F);
    EXPECT_GT(band.getWhiteningAvg(), 0.05F);

    band.reset();
    EXPECT_FLOAT_EQ(band.getBaseline(),     0.0F);
    EXPECT_FLOAT_EQ(band.getWhiteningAvg(), 0.0F);
}
