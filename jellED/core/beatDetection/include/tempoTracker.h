#ifndef __TEMPO_TRACKER_JELLED_H__
#define __TEMPO_TRACKER_JELLED_H__

#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

#include "ringbuffer.h"

namespace jellED {

class TempoTracker {

public:
    explicit TempoTracker(size_t maxHistory = 12,       // More = smoother but slower adaptation
        double minBpm = 70.0,                           // Absolute minimum BPM accepted
        double maxBpm = 200.0,                          // Absolute maximum BPM accepted
        double tolerance = 0.25);                       // ±25% for established tempo

    ~TempoTracker();

    void addBeat(double timestamp);

    double currentIbi() const;

    double currentBpm() const;

    bool isTempoConsistent(double candidateTime) const;

    bool hasEstablishedTempo() const { return beatTimes_->size() >= 2; }

    size_t beatCount() const { return beatTimes_->size(); }

    double tempoVariance() const;

    void setMinBpm(double minBpm) { minBpm_ = minBpm; }
    void setMaxBpm(double maxBpm) { maxBpm_ = maxBpm; }
    void setTolerance(double tolerance) { tolerance_ = tolerance; }

    // Drop all recorded beats so the tracker cold-starts on the next addBeat.
    // Used for staleness reset after long silences and for resetting state
    // between tests.
    void reset() { beatTimes_->clear(); }

private:
    Ringbuffer* beatTimes_;
    size_t maxHistory_;
    double minBpm_;
    double maxBpm_;
    double tolerance_;

    std::vector<double> collectIntervals() const;

    static double median(std::vector<double>& values);

    double defaultIbi() const;
};
} // end namespace jellED

#endif
