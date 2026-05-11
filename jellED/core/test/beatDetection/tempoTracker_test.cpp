#include "include/tempoTracker.h"

#include <gtest/gtest.h>

namespace {

constexpr double IBI_128_BPM = 60.0 / 128.0;  // ~0.46875 s per beat

}  // namespace

TEST(TempoTrackerTest, ColdStartPassesUnconditionally) {
    jellED::TempoTracker tracker(12, 90.0, 180.0, 0.20);
    EXPECT_FALSE(tracker.hasEstablishedTempo());
    // With zero beats recorded the gate must be permissive — otherwise the
    // first beat after enabling tempo lock could never establish the grid.
    EXPECT_TRUE(tracker.isTempoConsistent(0.0));
    EXPECT_TRUE(tracker.isTempoConsistent(10.0));
}

TEST(TempoTrackerTest, EstablishesTempoAfterTwoBeats) {
    jellED::TempoTracker tracker(12, 90.0, 180.0, 0.20);
    tracker.addBeat(0.0);
    EXPECT_FALSE(tracker.hasEstablishedTempo());
    tracker.addBeat(IBI_128_BPM);
    EXPECT_TRUE(tracker.hasEstablishedTempo());
    EXPECT_NEAR(tracker.currentBpm(), 128.0, 0.5);
}

TEST(TempoTrackerTest, AcceptsBeatsOnGrid) {
    jellED::TempoTracker tracker(12, 90.0, 180.0, 0.20);
    double t = 0.0;
    for (int i = 0; i < 6; ++i) {
        tracker.addBeat(t);
        t += IBI_128_BPM;
    }
    EXPECT_NEAR(tracker.currentBpm(), 128.0, 0.5);
    // Next on-grid beat must be accepted.
    EXPECT_TRUE(tracker.isTempoConsistent(t));
}

TEST(TempoTrackerTest, RejectsOffGridBeats) {
    jellED::TempoTracker tracker(12, 90.0, 180.0, 0.20);
    double t = 0.0;
    // Build > 6 beats so the strict tolerance applies (TempoTracker uses
    // ±35% during establishment, falling to the configured tolerance once
    // beatCount >= 6).
    for (int i = 0; i < 8; ++i) {
        tracker.addBeat(t);
        t += IBI_128_BPM;
    }
    // Halfway between grid points — interval ~half the IBI. Must reject.
    const double offGrid = t - IBI_128_BPM * 0.5;
    EXPECT_FALSE(tracker.isTempoConsistent(offGrid));
    // Slightly off but inside tolerance — must accept.
    EXPECT_TRUE(tracker.isTempoConsistent(t + 0.05));
}

TEST(TempoTrackerTest, RejectsAboveAbsoluteMaxBpm) {
    jellED::TempoTracker tracker(12, 90.0, 180.0, 0.20);
    tracker.addBeat(0.0);
    // 250 BPM is well above the configured maxBpm of 180. Must reject even
    // before tempo establishment thanks to the absolute-bounds check.
    EXPECT_FALSE(tracker.isTempoConsistent(60.0 / 250.0));
}

TEST(TempoTrackerTest, ResetClearsHistory) {
    jellED::TempoTracker tracker(12, 90.0, 180.0, 0.20);
    tracker.addBeat(0.0);
    tracker.addBeat(IBI_128_BPM);
    tracker.addBeat(2.0 * IBI_128_BPM);
    EXPECT_TRUE(tracker.hasEstablishedTempo());
    tracker.reset();
    EXPECT_FALSE(tracker.hasEstablishedTempo());
    EXPECT_EQ(tracker.beatCount(), 0u);
    // After reset the gate is permissive again.
    EXPECT_TRUE(tracker.isTempoConsistent(100.0));
}

TEST(TempoTrackerTest, ToleranceSetterChangesGateWidth) {
    jellED::TempoTracker tracker(12, 90.0, 180.0, 0.20);
    double t = 0.0;
    for (int i = 0; i < 8; ++i) {
        tracker.addBeat(t);
        t += IBI_128_BPM;
    }
    // ~25% off grid: rejected at tolerance 0.20, accepted at tolerance 0.40.
    const double candidate = t + IBI_128_BPM * 0.25;
    EXPECT_FALSE(tracker.isTempoConsistent(candidate));
    tracker.setTolerance(0.40);
    EXPECT_TRUE(tracker.isTempoConsistent(candidate));
}

TEST(TempoTrackerTest, MaxBpmSetterTightensAbsoluteBounds) {
    jellED::TempoTracker tracker(12, 60.0, 240.0, 0.20);
    tracker.addBeat(0.0);
    // 200 BPM accepted under the original 240 ceiling.
    EXPECT_TRUE(tracker.isTempoConsistent(60.0 / 200.0));
    tracker.setMaxBpm(150.0);
    // Same candidate now violates the lower minIbi bound and must reject.
    EXPECT_FALSE(tracker.isTempoConsistent(60.0 / 200.0));
}
