#ifndef __WAVEFORMWIDGET_H__
#define __WAVEFORMWIDGET_H__

#include <QWidget>
#include <QThread>
#include <QPainter>
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
    std::unique_ptr<jellED::Ringbuffer> windowedSampleRingBuffer_;

    int bufferWriteIndex_;

    QPen waveformPen_;
    QPen gridPen_;
    QPen centerLinePen_;

    double currentWindowMin_;
    double currentWindowMax_;

    std::vector<WaveformPoint> windowedSampleData_;
    std::mutex dataMutex_;
    void drawGrid(QPainter& painter);

protected:
    int sampleRate_;
    const int displaySeconds_;
    int numSamplesInWindow_;
    int numSamplesAdded_;
    virtual void drawWaveform(QPainter& painter);
    void paintEvent(QPaintEvent* event) override;

public:
    WaveformWidget(int sampleRate, int displaySeconds, QWidget* parent = nullptr);
    void updateWidget();
    virtual void addSample(const double sample);
    virtual void clearSamples();
};

#endif