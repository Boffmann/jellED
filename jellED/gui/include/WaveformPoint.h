#ifndef __WAVEFORMPOINT_H__
#define __WAVEFORMPOINT_H__

#include <QPointF>

/**
 * Min/Max pair for efficient waveform display
 */
 struct WaveformPoint {
    float min;
    float max;
    
    WaveformPoint() : min(0.0f), max(0.0f) {}
    WaveformPoint(float mn, float mx) : min(mn), max(mx) {}
};

// Declare the type for Qt's meta-object system
Q_DECLARE_METATYPE(std::vector<WaveformPoint>)

#endif