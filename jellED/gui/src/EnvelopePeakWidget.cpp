#include "EnvelopePeakWidget.h"
#include <QPainter>
#include <iostream>

EnvelopePeakWidget::EnvelopePeakWidget(int sampleRate, int displaySeconds, QWidget* parent)
    : WaveformWidget(sampleRate, displaySeconds, parent)
    , peakColor_(255, 0, 0)
{
    ringPeakBuffer_ = std::make_unique<jellED::Ringbuffer>(sampleRate_ * displaySeconds_);
    ringPeakBuffer_->fill(0.0);
}

void EnvelopePeakWidget::paintEvent(QPaintEvent* event) {
    WaveformWidget::paintEvent(event);
}

void EnvelopePeakWidget::addSample(const double sample) {
    std::lock_guard<std::mutex> lock(peakDataMutex_);
    WaveformWidget::addSample(sample);
    ringPeakBuffer_->append(0.0);
}

void EnvelopePeakWidget::drawWaveform(QPainter& painter) {
    std::lock_guard<std::mutex> lock(peakDataMutex_);
    WaveformWidget::drawWaveform(painter);
    ringPeakBuffer_->size();
    if (ringPeakBuffer_->size() == 0) {
        return;
    }

    painter.setPen(QPen(peakColor_, 1.0));

    int w = width();
    int h = height();
    double centerY = h / 2.0f;
    double scale = centerY * 0.9f;  // 0.9 for some margin

    for (size_t i = 0; i < w; ++i) {
        float x = static_cast<float>(i);
        WaveformPoint currentPoint = scaleSampleData(i, ringPeakBuffer_);
        if (currentPoint.min == 0.0 && currentPoint.max == 0.0) {
            continue;
        }
        // Clamp values
        float minVal = std::max(-1.0f, std::min(1.0f, currentPoint.min));
        float maxVal = std::max(-1.0f, std::min(1.0f, currentPoint.max));
        
        // Convert to screen coordinates (invert Y because Qt coordinates are top-down)
        float yMin = centerY - (maxVal * scale);
        float yMax = centerY - (minVal * scale);
        
        // Draw vertical line from min to max
        painter.drawLine(QPointF(x, yMin), QPointF(x, yMax));
    }
}

void EnvelopePeakWidget::addPeak() {
    std::lock_guard<std::mutex> lock(peakDataMutex_);
    std::cout << "Adding peak" << std::endl;
    ringPeakBuffer_->override_head_value(1.0);
    //ringPeakBuffer_->append(1.0);
}