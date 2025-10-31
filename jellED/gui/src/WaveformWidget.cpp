#include "WaveformWidget.h"
#include <QPainter>
#include <iostream>

WaveformWidget::WaveformWidget(int sampleRate, int displaySeconds, QWidget* parent)
    : QWidget(parent)
    , backgroundColor_(Qt::black)
    , waveformColor_(0, 255, 0)  // Green
    , gridColor_(40, 40, 40)
    , sampleRate_(sampleRate)
    , displaySeconds_(displaySeconds)
{
    setMinimumSize(400, 200);

    ringSampleBuffer_ = std::make_unique<jellED::Ringbuffer>(sampleRate_ * displaySeconds_);
    ringSampleBuffer_->fill(0.0);
}

void WaveformWidget::addSample(const double sample) {
    std::lock_guard<std::mutex> lock(dataMutex_);
    ringSampleBuffer_->append(sample);
}

void WaveformWidget::updateWidget() {
    update();
}

void WaveformWidget::clearSamples() {
    std::lock_guard<std::mutex> lock(dataMutex_);
    ringSampleBuffer_->fill(0.0);
}

void WaveformWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw background
    painter.fillRect(rect(), backgroundColor_);
    drawGrid(painter);

    drawWaveform(painter);
}

void WaveformWidget::drawWaveform(QPainter& painter) {
    std::lock_guard<std::mutex> lock(dataMutex_);
    
    if (ringSampleBuffer_->size() == 0) {
        return;
    }
    
    int w = width();
    int h = height();
    double centerY = h / 2.0f;
    double scale = centerY * 0.9f;  // 0.9 for some margin
    
    painter.setPen(QPen(waveformColor_, 1.0));
    
    // Draw using vertical lines for each pixel column
    // This is much faster than QPainterPath for dense waveforms
    for (size_t i = 0; i < w; ++i) {
        float x = static_cast<float>(i);
        WaveformPoint currentPoint = scaleSampleData(i, ringSampleBuffer_);
        
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

void WaveformWidget::drawGrid(QPainter& painter) {
    painter.setPen(QPen(gridColor_, 1));
    
    int w = width();
    int h = height();
    int centerY = h / 2;
    
    // Draw horizontal center line (0 amplitude)
    painter.setPen(QPen(gridColor_.lighter(150), 1));
    painter.drawLine(0, centerY, w, centerY);
    
    // Draw horizontal grid lines
    painter.setPen(QPen(gridColor_, 1));
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

WaveformPoint WaveformWidget::scaleSampleData(const uint32_t width_index, const std::unique_ptr<jellED::Ringbuffer>& ringBuffer) const {
    if (ringBuffer->size() == 0) {
        return WaveformPoint(0.0f, 0.0f);
    }
    
    int w = width();
    if (w <= 0) {
        return WaveformPoint(0.0f, 0.0f);
    }
    
    size_t numSamples = ringBuffer->size();
    
    // Calculate how many samples per pixel
    double samplesPerPixel = static_cast<double>(numSamples) / w;
    
    if (samplesPerPixel <= 1.0f) {
            size_t sampleIdx = static_cast<size_t>((width_index * numSamples) / w);
            if (sampleIdx < numSamples) {
                double sample = ringBuffer->get(sampleIdx);
                return WaveformPoint(sample, sample);
            } else {
                return WaveformPoint(0.0f, 0.0f);
            }
    } else {
        // More samples than pixels - need min/max downsampling
            size_t startSample = static_cast<size_t>(width_index * samplesPerPixel);
            size_t endSample = static_cast<size_t>((width_index + 1) * samplesPerPixel);
            
            if (endSample > numSamples) {
                endSample = numSamples;
            }
            
            // Find min and max in this range
            double minVal = ringBuffer->get(startSample);
            double maxVal = ringBuffer->get(startSample);
            
            for (size_t i = startSample + 1; i < endSample; ++i) {
                double sample = ringBuffer->get(i);
                if (sample < minVal) minVal = sample;
                if (sample > maxVal) maxVal = sample;
            }
            
            return WaveformPoint(minVal, maxVal);
    }
}
