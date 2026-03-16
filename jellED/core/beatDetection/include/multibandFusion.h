#ifndef __MULTIBAND_FUSION_JELLED_H__
#define __MULTIBAND_FUSION_JELLED_H__

#include <cmath>
#include <algorithm>

namespace jellED {

enum class BAND_ID : int {
    LOW,
    MID,
    HIGH
};

struct BandPeak {
    float time;
    float strength;
    BAND_ID bandId;
};

class MultiBandFusion {
public:
    // Max peaks expected in coincidence window (~50ms)
    // 3 bands × ~3 peaks each = 9, round up for safety
    static constexpr size_t MAX_PEAKS = 16;

    explicit MultiBandFusion(float coincidenceWindow, float maxBpm);

    bool push(const BandPeak& peak);

    void setCoincidenceWindow(float window);
    void setMaxBpm(float maxBpm);

private:
    float coincidenceWindow_;
    float lastBeatTime_;
    float minBeatInterval_;
    
    // Fixed-size circular buffer for peaks
    BandPeak peaks_[MAX_PEAKS];
    size_t head_;       // Index of oldest element
    size_t count_;      // Number of valid elements
    
    void removeExpiredPeaks(float currentTime);
    void addPeak(const BandPeak& peak);
};

} // namespace jellED

#endif
