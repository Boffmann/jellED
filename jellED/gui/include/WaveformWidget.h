#ifndef __WAVEFORMWIDGET_H__
#define __WAVEFORMWIDGET_H__

#include <QWidget>
#include <QThread>
#include <vector>
#include <mutex>

#include "include/ringbuffer.h"

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

class WaveformWidget : public QWidget {
    Q_OBJECT

private:
    QColor backgroundColor_;
    QColor waveformColor_;
    QColor gridColor_;
    std::unique_ptr<jellED::Ringbuffer> ringSampleBuffer_;
    std::mutex dataMutex_;
    const int sampleRate_;
    const int displaySeconds_;
    void drawGrid(QPainter& painter);
    void drawWaveform(QPainter& painter);
    WaveformPoint scaleSampleData(const uint32_t index) const;

protected:
    void paintEvent(QPaintEvent* event) override;

public:
    WaveformWidget(int sampleRate, int displaySeconds, QWidget* parent = nullptr);
    void updateWidget();
    void clearSamples();
    void addSample(const double sample);
};

#endif