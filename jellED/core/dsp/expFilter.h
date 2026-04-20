#ifndef _JELLED_EXP_FILTER_H_
#define _JELLED_EXP_FILTER_H_

#include <cmath>

namespace jellED {

// Time-aware asymmetric exponential smoothing filter (first-order IIR low-pass).
//
// Each update blends the new input with the stored state using a coefficient
// computed from elapsed time and the configured time constants. Different
// time constants can be used for rising vs. falling inputs, giving the filter
// a distinct "personality":
//   tau_rise << tau_decay  → peak follower / envelope detector
//   tau_rise >> tau_decay  → baseline / common-mode tracker
//   tau_rise == tau_decay  → ordinary low-pass smoother
//
// Time-aware: α is recomputed each update from dt, so the filter's behaviour
// stays consistent even if the call rate varies. Patterns declare intent in
// human time (e.g. tau_rise = 30ms) rather than in α values tied to a fixed
// sample rate.
//
// See audio_reactive_led_strip_learnings.md §4.1 for the full rationale.
//
// Typical usage (asymmetric peak follower):
//   ExpFilter<float> brightness(0.0f, /*tau_rise*/ 0.03f, /*tau_decay*/ 0.5f);
//   // ... per frame, with dt_seconds since previous update:
//   float smoothed = brightness.update(target, dt_seconds);
//
// Contract: tau_rise_s and tau_decay_s must be > 0. dt_seconds must be >= 0.
// T is expected to be a floating-point type (float or double).
template <typename T>
class ExpFilter {
public:
    ExpFilter(T initial, float tau_rise_s, float tau_decay_s)
        : value_(initial), tau_rise_(tau_rise_s), tau_decay_(tau_decay_s) {}

    // Apply one update. Returns the new filtered value.
    T update(T input, float dt_seconds) {
        const float tau = (input > value_) ? tau_rise_ : tau_decay_;
        const float alpha = 1.0f - std::exp(-dt_seconds / tau);
        value_ = alpha * input + (1.0f - alpha) * value_;
        return value_;
    }

    // Read current filter state without updating.
    T value() const { return value_; }

    // Force internal state to a value (e.g. after a discontinuous jump in
    // semantics where smoothing from the old state would be wrong).
    void reset(T new_value) { value_ = new_value; }

    // Change time constants at runtime. New α is computed on the next update.
    void setTimeConstants(float tau_rise_s, float tau_decay_s) {
        tau_rise_ = tau_rise_s;
        tau_decay_ = tau_decay_s;
    }

    float getTauRise() const { return tau_rise_; }
    float getTauDecay() const { return tau_decay_; }

    // Compute the blend coefficient α for a given time constant and elapsed
    // time step. Useful when decaying many independent values with a shared
    // tau (e.g. a sparkle array) — avoids allocating a filter per element
    // while keeping the same time-aware semantics.
    static float alphaFromDt(float tau_seconds, float dt_seconds) {
        return 1.0f - std::exp(-dt_seconds / tau_seconds);
    }

private:
    T value_;
    float tau_rise_;
    float tau_decay_;
};

} // namespace jellED

#endif
