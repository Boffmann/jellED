#ifndef __WAVEFORMWIDGET_H__
#define __WAVEFORMWIDGET_H__

#include <QWidget>
#include <QThread>
#include <vector>
#include <mutex>

#include "include/ringbuffer.h"
#include "WaveformPoint.h"

class WaveformWidget : public QWidget {
    Q_OBJECT

private:
    QColor backgroundColor_;
    QColor waveformColor_;
    QColor gridColor_;
    std::unique_ptr<jellED::Ringbuffer> ringSampleBuffer_;
    std::mutex dataMutex_;
    void drawGrid(QPainter& painter);

protected:
    const int sampleRate_;
    const int displaySeconds_;
    virtual void drawWaveform(QPainter& painter);
    WaveformPoint scaleSampleData(const uint32_t index, const std::unique_ptr<jellED::Ringbuffer>& ringBuffer) const;
    void paintEvent(QPaintEvent* event) override;

public:
    WaveformWidget(int sampleRate, int displaySeconds, QWidget* parent = nullptr);
    void updateWidget();
    void clearSamples();
    virtual void addSample(const double sample);
};

#endif