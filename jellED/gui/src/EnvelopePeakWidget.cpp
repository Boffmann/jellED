#include "EnvelopePeakWidget.h"
#include <QPainter>
#include <iostream>

EnvelopePeakWidget::EnvelopePeakWidget(int sampleRate, int displaySeconds, QWidget* parent)
    : WaveformWidget(sampleRate, displaySeconds, parent)
    , peakColor_(255, 0, 0)
    , peakPen_(peakColor_, 1.0)
    , thresholdPen_(QColor(255, 165, 0), 1.0)
    , peakWriteIndex_(0)
    , currentThreshold_(0.0)
{
    windowedPeakRingBuffer_ = std::make_unique<jellED::Ringbuffer>(numSamplesInWindow_);
    windowedPeakRingBuffer_->fill(0.0);

    windowedPeakData_.resize(width(), WaveformPoint(0.0f, 0.0f));
    windowedThresholdData_.resize(width(), 0.0f);
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
    windowedThresholdData_.resize(width(), 0.0f);
    peakWriteIndex_ = 0;
}

void EnvelopePeakWidget::setCurrentThreshold(double threshold) {
    currentThreshold_ = threshold;
}

void EnvelopePeakWidget::addSample(const double sample) {
    int prevSamplesAdded = numSamplesAdded_ + 1;

    WaveformWidget::addSample(sample);
    
    std::lock_guard<std::mutex> lock(peakDataMutex_);
    windowedPeakRingBuffer_->append(0.0);
    
    if (prevSamplesAdded > 0 && numSamplesAdded_ == 0) {
        windowedPeakData_[peakWriteIndex_] = WaveformPoint(0.0f, 0.0f);
        windowedThresholdData_[peakWriteIndex_] = static_cast<float>(currentThreshold_);
        peakWriteIndex_ = (peakWriteIndex_ + 1) % width();
    }
}

void EnvelopePeakWidget::clearSamples() {
    std::lock_guard<std::mutex> lock(peakDataMutex_);
    WaveformWidget::clearSamples();
    windowedPeakRingBuffer_->fill(0.0);
    std::fill(windowedPeakData_.begin(), windowedPeakData_.end(), WaveformPoint(0.0f, 0.0f));
    std::fill(windowedThresholdData_.begin(), windowedThresholdData_.end(), 0.0f);
    peakWriteIndex_ = 0;
    currentThreshold_ = 0.0;
}

void EnvelopePeakWidget::drawWaveform(QPainter& painter) {
    WaveformWidget::drawWaveform(painter);

    std::lock_guard<std::mutex> lock(peakDataMutex_);
    if (windowedPeakRingBuffer_->size() == 0) {
        return;
    }

    int w = width();
    int h = height();
    double centerY = h / 2.0f;
    double scale = centerY * 0.9f;

    // Draw threshold line (orange, connected line segments)
    drawThresholdLine(painter);

    // Draw peak markers (red vertical bars)
    painter.setPen(peakPen_);

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
        
        peakLines.append(QLineF(i, yMin, i, yMax));
    }
    painter.drawLines(peakLines);
}

void EnvelopePeakWidget::drawThresholdLine(QPainter& painter) {
    painter.setPen(thresholdPen_);

    int w = width();
    int h = height();
    double centerY = h / 2.0;
    double scale = centerY * 0.9;

    QVector<QLineF> thresholdLines;
    thresholdLines.reserve(w);

    float prevY = -1.0f;
    for (int i = 0; i < w; ++i) {
        int bufferIndex = (peakWriteIndex_ + i) % w;
        float threshold = windowedThresholdData_[bufferIndex];

        if (threshold <= 0.0f) {
            prevY = -1.0f;
            continue;
        }

        float y = static_cast<float>(centerY - (threshold * scale));
        if (prevY >= 0.0f) {
            thresholdLines.append(QLineF(i - 1, prevY, i, y));
        }
        prevY = y;
    }
    painter.drawLines(thresholdLines);
}

void EnvelopePeakWidget::addPeak() {
    std::lock_guard<std::mutex> lock(peakDataMutex_);
    int currentPos = (peakWriteIndex_ - 1 + width()) % width();
    windowedPeakData_[currentPos] = WaveformPoint(0.0f, 1.0f);
}
