#include "LedStripWidget.h"

#include <QPainter>
#include <algorithm>

LedStripWidget::LedStripWidget(int numLeds, QWidget* parent)
    : QWidget(parent),
      numLeds_(numLeds),
      pending_(numLeds, jellED::pattern_color{0, 0, 0, 0}),
      displayed_(numLeds, jellED::pattern_color{0, 0, 0, 0}),
      pendingDirty_(false) {
    setMinimumHeight(70);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setAutoFillBackground(false);

    connect(&refreshTimer_, &QTimer::timeout, this, &LedStripWidget::onRefreshTimer);
    refreshTimer_.start(16); // ~60 Hz; decoupled from pattern update cadence
}

void LedStripWidget::setColors(const jellED::pattern_color* colors, int count) {
    const int n = std::min(count, numLeds_);
    std::lock_guard<std::mutex> lock(mutex_);
    for (int i = 0; i < n; ++i) {
        pending_[i] = colors[i];
    }
    pendingDirty_ = true;
}

void LedStripWidget::onRefreshTimer() {
    bool dirty = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (pendingDirty_) {
            displayed_ = pending_;
            pendingDirty_ = false;
            dirty = true;
        }
    }
    if (dirty) {
        update();
    }
}

void LedStripWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), QColor(20, 20, 20));

    if (numLeds_ <= 0) return;

    const qreal cellW = static_cast<qreal>(width()) / numLeds_;
    const qreal padding = 4.0;
    const qreal diameter =
        std::min(cellW - 2.0 * padding, static_cast<qreal>(height()) - 2.0 * padding);
    if (diameter <= 0.0) return;

    p.setPen(Qt::NoPen);
    for (int i = 0; i < numLeds_; ++i) {
        const jellED::pattern_color& c = displayed_[i];
        // brightness is the WS2812 "global" channel — fold it into alpha so a
        // black LED (brightness=0) actually appears dark in the widget.
        const int alpha = static_cast<int>(c.brightness);
        p.setBrush(QColor(c.red, c.green, c.blue, alpha));
        const qreal cx = (i + 0.5) * cellW;
        const qreal cy = height() * 0.5;
        p.drawEllipse(QPointF(cx, cy), diameter * 0.5, diameter * 0.5);
    }
}
