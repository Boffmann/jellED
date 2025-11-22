#include "EnvelopePeakWidget.h"
#include <QPainter>
#include <iostream>

EnvelopePeakWidget::EnvelopePeakWidget(int sampleRate, int displaySeconds, QWidget* parent)
    : WaveformWidget(sampleRate, displaySeconds, parent)
    , peakColor_(255, 0, 0)
    , peakPen_(peakColor_, 1.0)
    , peakWriteIndex_(0)
{
    windowedPeakRingBuffer_ = std::make_unique<jellED::Ringbuffer>(numSamplesInWindow_);
    windowedPeakRingBuffer_->fill(0.0);

    windowedPeakData_.resize(width(), WaveformPoint(0.0f, 0.0f));
}

void EnvelopePeakWidget::paintEvent(QPaintEvent* event) {
    WaveformWidget::paintEvent(event);
}

void EnvelopePeakWidget::resizeEvent(QResizeEvent* event) {
    std::lock_guard<std::mutex> lock(peakDataMutex_);
    WaveformWidget::resizeEvent(event);
    windowedPeakRingBuffer_ = std::make_unique<jellED::Ringbuffer>(numSamplesInWindow_);
    windowedPeakRingBuffer_->fill(0.0);
    windowedPeakData_.resize(width(), WaveformPoint(0.0f, 0.0f));
    peakWriteIndex_ = 0;
}

void EnvelopePeakWidget::addSample(const double sample) {
    int prevSamplesAdded = numSamplesAdded_ + 1;

    WaveformWidget::addSample(sample);
    
    std::lock_guard<std::mutex> lock(peakDataMutex_);
    windowedPeakRingBuffer_->append(0.0);
    
    if (prevSamplesAdded > 0 && numSamplesAdded_ == 0) {
        windowedPeakData_[peakWriteIndex_] = WaveformPoint(0.0f, 0.0f);
        peakWriteIndex_ = (peakWriteIndex_ + 1) % width();
    }
}

void EnvelopePeakWidget::clearSamples() {
    std::lock_guard<std::mutex> lock(peakDataMutex_);
    WaveformWidget::clearSamples();
    windowedPeakRingBuffer_->fill(0.0);
    std::fill(windowedPeakData_.begin(), windowedPeakData_.end(), WaveformPoint(0.0f, 0.0f));
    peakWriteIndex_ = 0;
}

void EnvelopePeakWidget::drawWaveform(QPainter& painter) {
    WaveformWidget::drawWaveform(painter);

    std::lock_guard<std::mutex> lock(peakDataMutex_);
    if (windowedPeakRingBuffer_->size() == 0) {
        return;
    }

    painter.setPen(peakPen_);

    int w = width();
    int h = height();
    double centerY = h / 2.0f;
    double scale = centerY * 0.9f;  // 0.9 for some margin

    QVector<QLineF> peakLines;
    peakLines.reserve(w);

    for (int i = 0; i < w; ++i) {
        int bufferIndex = (peakWriteIndex_ + i) % w;
        WaveformPoint currentPoint = windowedPeakData_[bufferIndex];
        
        if (currentPoint.min == 0.0 && currentPoint.max == 0.0) {
            continue;
        }
        
        float minVal = std::max(-1.0f, std::min(1.0f, currentPoint.min));
        float maxVal = std::max(-1.0f, std::min(1.0f, currentPoint.max));
        
        float yMin = centerY - (maxVal * scale);
        float yMax = centerY - (minVal * scale);
        
        // painter.drawLine(QPointF(i, yMin), QPointF(i, yMax));
        peakLines.append(QLineF(i, yMin, i, yMax));
    }
    painter.drawLines(peakLines);
}

void EnvelopePeakWidget::addPeak() {
    std::lock_guard<std::mutex> lock(peakDataMutex_);
    // std::cout << "Adding peak" << std::endl;
    int currentPos = (peakWriteIndex_ - 1 + width()) % width();
    windowedPeakData_[currentPos] = WaveformPoint(0.0f, 1.0f);
}
