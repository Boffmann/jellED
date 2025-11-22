#ifndef __MULTIBAND_FUSION_JELLED_H__
#define __MULTIBAND_FUSION_JELLED_H__

#include <deque>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

struct BandPeak {
    double time;
    double strength;
    int bandId;  // 0=LOW, 1=MID, 2=HIGH
};

class MultiBandFusion {
    public:
        explicit MultiBandFusion(double coincidenceWindow, double maxBpm);
    
        bool push(const BandPeak& peak);
    
    private:
        double coincidenceWindow_;  // Time window for multi-band coincidence
        double lastBeatTime_;
        double minBeatInterval_;
        std::deque<BandPeak> peaks_;
};

#endif
