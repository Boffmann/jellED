#include "include/tempoTracker.h"

#include <algorithm>
#include <iostream>
#include <cmath>

namespace jellED {

TempoTracker::TempoTracker(size_t maxHistory, double minBpm, double maxBpm, double tolerance)
    : maxHistory_(maxHistory),
      minBpm_(minBpm),
      maxBpm_(maxBpm),
      tolerance_(tolerance) {
        beatTimes_ = new Ringbuffer(maxHistory_);
        beatTimes_->clear();  // Reset to empty state (Ringbuffer constructor pre-fills with zeros)
      }

TempoTracker::~TempoTracker() {
    delete beatTimes_;
}

void TempoTracker::addBeat(double timestamp) {
    if (beatTimes_->size() == 0 || timestamp > beatTimes_->get(beatTimes_->size() - 1)) {
        beatTimes_->append(timestamp);
    }
}

double TempoTracker::currentIbi() const {
    auto intervals = collectIntervals();
    if (intervals.empty()) {
        return defaultIbi();
    }
    return median(intervals);
}

double TempoTracker::currentBpm() const {
    double ibi = currentIbi();
    if (ibi <= 0.0) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    return 60.0 / ibi;
}

bool TempoTracker::isTempoConsistent(double candidateTime) const {
    if (beatTimes_->size() == 0) {
        return true;
    }
    double ibi = currentIbi();
    double minIbi = 60.0 / maxBpm_;
    double maxIbi = 60.0 / minBpm_;

    double interval = candidateTime - beatTimes_->get(beatTimes_->size() - 1);
    
    // First check: Must be within absolute BPM limits
    if (interval < minIbi * (1.0 - tolerance_)) {
        return false;
    }
    if (interval > maxIbi * (1.0 + tolerance_)) {
        return false;
    }
    
    // Second check: Adaptive tolerance based on tempo stability
    // If we have few beats, be more lenient to allow tempo to establish
    // If we have many beats, use standard tolerance
    double adaptiveTolerance = tolerance_;
    if (beatTimes_->size() < 6) {
        // During tempo establishment or change, be more lenient (±35%)
        adaptiveTolerance = 0.35;
    }
    
    double lowerBound = ibi * (1.0 - adaptiveTolerance);
    double upperBound = ibi * (1.0 + adaptiveTolerance);
    bool consistent = interval >= lowerBound && interval <= upperBound;
    return consistent;
}


std::vector<double> TempoTracker::collectIntervals() const {
    std::vector<double> intervals;
    if (beatTimes_->size() < 2) {
        return intervals;
    }
    intervals.reserve(beatTimes_->size() - 1);
    for (size_t i = 1; i < beatTimes_->size(); ++i) {
        intervals.push_back(beatTimes_->get(i) - beatTimes_->get(i - 1));
    }
    return intervals;
}

double TempoTracker::median(std::vector<double>& values) {
    size_t n = values.size();
    size_t mid = n / 2;
    std::nth_element(values.begin(), values.begin() + mid, values.end());
    double med = values[mid];
    if (n % 2 == 0) {
        std::nth_element(values.begin(), values.begin() + mid - 1, values.end());
        med = 0.5 * (med + values[mid - 1]);
    }
    return med;
}

double TempoTracker::defaultIbi() const {
    double bpm = (minBpm_ + maxBpm_) * 0.5;
    return 60.0 / bpm;
}

double TempoTracker::tempoVariance() const {
    auto intervals = collectIntervals();
    if (intervals.size() < 2) {
        return 0.0;
    }
    
    double ibi = currentIbi();
    double variance = 0.0;
    for (double interval : intervals) {
        double diff = interval - ibi;
        variance += diff * diff;
    }
    return variance / intervals.size();
}

} // end namespace jellED
