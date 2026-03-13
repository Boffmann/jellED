#ifndef __ENVELOPEPEAKWIDGET_H__
#define __ENVELOPEPEAKWIDGET_H__

#include <QWidget>
#include <QThread>
#include <vector>
#include <mutex>

#include "include/ringbuffer.h"
#include "WaveformWidget.h"
#include "WaveformPoint.h"

class EnvelopePeakWidget : public WaveformWidget {

private:
    QColor peakColor_;
    std::unique_ptr<jellED::Ringbuffer> windowedPeakRingBuffer_;
    std::vector<WaveformPoint> windowedPeakData_;
    QPen peakPen_;
    QPen thresholdPen_;
    int peakWriteIndex_;
    std::mutex peakDataMutex_;

    double currentThreshold_;
    std::vector<float> windowedThresholdData_;

    void drawThresholdLine(QPainter& painter);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void drawWaveform(QPainter& painter) override;

public:
    EnvelopePeakWidget(int sampleRate, int displaySeconds, QWidget* parent = nullptr);
    void addSample(const double sample) override;
    void clearSamples() override;
    void addPeak();

    // Call before addSample() for each sample to keep threshold in sync
    void setCurrentThreshold(double threshold);
};

#endif
