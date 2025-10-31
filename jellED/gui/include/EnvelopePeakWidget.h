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
    std::unique_ptr<jellED::Ringbuffer> ringPeakBuffer_;
    std::mutex peakDataMutex_;

protected:
    void paintEvent(QPaintEvent* event) override;
    void drawWaveform(QPainter& painter) override;

public:
    EnvelopePeakWidget(int sampleRate, int displaySeconds, QWidget* parent = nullptr);
    void addSample(const double sample) override;
    void addPeak();
};

#endif