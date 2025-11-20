#include "WaveformWidget.h"
#include <iostream>

WaveformWidget::WaveformWidget(int sampleRate, int displaySeconds, QWidget* parent)
    : QWidget(parent)
    , backgroundColor_(Qt::black)
    , waveformColor_(0, 255, 0)  // Green
    , gridColor_(40, 40, 40)
    , bufferWriteIndex_(0)
    , waveformPen_(waveformColor_, 1.0)
    , gridPen_(gridColor_, 1)
    , centerLinePen_(gridColor_.lighter(150), 1)
    , currentWindowMin_(0.0)
    , currentWindowMax_(0.0)
    , sampleRate_(sampleRate)
    , displaySeconds_(displaySeconds)
    , numSamplesInWindow_(0)
    , numSamplesAdded_(0)
{
    setMinimumSize(400, 200);

    setAttribute(Qt::WA_OpaquePaintEvent);  // We paint everything, no need for background
    setAttribute(Qt::WA_NoSystemBackground); // Skip system background
    setAttribute(Qt::WA_StaticContents);     // Content doesn't scroll

    numSamplesInWindow_ = static_cast<double>(sampleRate_ * displaySeconds_) / width();

    windowedSampleRingBuffer_ = std::make_unique<jellED::Ringbuffer>(numSamplesInWindow_);
    windowedSampleRingBuffer_->fill(0.0);

    windowedSampleData_.resize(width(), WaveformPoint(0.0f, 0.0f));
}

void WaveformWidget::addSample(const double sample) {
    std::lock_guard<std::mutex> lock(dataMutex_);
    windowedSampleRingBuffer_->append(sample);
    
    // Track min/max incrementally
    if (numSamplesAdded_ == 0) {
        currentWindowMin_ = sample;
        currentWindowMax_ = sample;
    } else {
        if (sample < currentWindowMin_) currentWindowMin_ = sample;
        if (sample > currentWindowMax_) currentWindowMax_ = sample;
    }
    
    numSamplesAdded_++;
    if (numSamplesAdded_ >= numSamplesInWindow_) {
        numSamplesAdded_ = 0;
        // Use tracked min/max instead of iterating
        windowedSampleData_[bufferWriteIndex_] = WaveformPoint(currentWindowMin_, currentWindowMax_);
        bufferWriteIndex_ = (bufferWriteIndex_ + 1) % width();
        // Reset for next window
        currentWindowMin_ = 0.0;
        currentWindowMax_ = 0.0;
    }
}

void WaveformWidget::updateWidget() {
    update();
}

void WaveformWidget::clearSamples() {
    std::lock_guard<std::mutex> lock(dataMutex_);
    windowedSampleRingBuffer_->fill(0.0);
    std::fill(windowedSampleData_.begin(), windowedSampleData_.end(), WaveformPoint(0.0f, 0.0f));
    numSamplesAdded_ = 0;
    bufferWriteIndex_ = 0;
}

void WaveformWidget::paintEvent(QPaintEvent* event) {
    (void)event;
    QPainter painter(this);
    
    // Draw background
    painter.fillRect(rect(), backgroundColor_);
    // drawGrid(painter);

    drawWaveform(painter);
}

void WaveformWidget::drawWaveform(QPainter& painter) {
    std::lock_guard<std::mutex> lock(dataMutex_);
    
    if (windowedSampleRingBuffer_->size() == 0) {
        return;
    }
    
    int w = width();
    int h = height();
    double centerY = h / 2.0f;
    double scale = centerY * 0.9f;  // 0.9 for some margin
    
    painter.setPen(waveformPen_);

    QVector<QLineF> lines;
    lines.reserve(w);
    
    // Draw using vertical lines for each pixel column
    // This is much faster than QPainterPath for dense waveforms
    for (int i = 0; i < w; ++i) {
        int bufferIndex = (bufferWriteIndex_ + i) % w;
        WaveformPoint currentPoint = windowedSampleData_[bufferIndex];

        // Clamp values
        float minVal = std::max(-1.0f, std::min(1.0f, currentPoint.min));
        float maxVal = std::max(-1.0f, std::min(1.0f, currentPoint.max));
        
        // Convert to screen coordinates (invert Y because Qt coordinates are top-down)
        float yMin = centerY - (maxVal * scale);
        float yMax = centerY - (minVal * scale);

        lines.append(QLineF(i, yMin, i, yMax));
        
        // Draw vertical line from min to max
        // painter.drawLine(QPointF(i, yMin), QPointF(i, yMax));
    }
    painter.drawLines(lines);
}

void WaveformWidget::drawGrid(QPainter& painter) {
    
    int w = width();
    int h = height();
    int centerY = h / 2;
    
    // Draw horizontal center line (0 amplitude)
    painter.setPen(centerLinePen_);
    painter.drawLine(0, centerY, w, centerY);
    
    // Draw horizontal grid lines
    painter.setPen(gridPen_);
    int gridSpacing = h / 8;
    for (int i = 1; i <= 4; ++i) {
        int y = centerY + i * gridSpacing;
        if (y < h) {
            painter.drawLine(0, y, w, y);
        }
        y = centerY - i * gridSpacing;
        if (y > 0) {
            painter.drawLine(0, y, w, y);
        }
    }
    
    // Draw vertical grid lines
    int numVerticalLines = this->displaySeconds_;
    for (int i = 1; i < numVerticalLines; ++i) {
        int x = (w * i) / numVerticalLines;
        painter.drawLine(x, 0, x, h);
    }
}
