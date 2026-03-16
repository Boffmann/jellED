#include "VolumeDisplayWidget.h"

#include <QPainter>
#include <algorithm>

static constexpr int LED_DIAMETER = 16;
static constexpr int LED_GAP      = 4;
static constexpr int MARGIN       = 6;
static constexpr int LABEL_HEIGHT = 18;
static constexpr int N_LEDS       = 12;  // must match VolumeDisplayWidget::NUM_LEDS

static constexpr int WIDGET_WIDTH  = MARGIN * 2 + LED_DIAMETER;
static constexpr int WIDGET_HEIGHT = MARGIN + N_LEDS * (LED_DIAMETER + LED_GAP)
                                     - LED_GAP + MARGIN + LABEL_HEIGHT;

VolumeDisplayWidget::VolumeDisplayWidget(const QString& label, QWidget* parent)
    : QWidget(parent)
    , label_(label)
    , currentValue_(0.0)
    , runningMax_(1e-9)
{
    setFixedSize(WIDGET_WIDTH, WIDGET_HEIGHT);
}

void VolumeDisplayWidget::setValue(double rawValue) {
    currentValue_ = rawValue;
    if (rawValue > runningMax_) {
        runningMax_ = rawValue;
    }
    update();
}

void VolumeDisplayWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor(20, 20, 20));

    const double norm = std::min(currentValue_ / runningMax_, 1.0);
    const int litLeds = static_cast<int>(norm * NUM_LEDS);

    for (int i = 0; i < NUM_LEDS; ++i) {
        // i=0 is the bottom LED, i=NUM_LEDS-1 is the top
        const int ledIndex = NUM_LEDS - 1 - i;  // draw top-to-bottom
        const int y = MARGIN + i * (LED_DIAMETER + LED_GAP);

        const bool lit = ledIndex < litLeds;

        QColor color;
        if (ledIndex < GREEN_LEDS) {
            color = lit ? QColor(50, 220, 50) : QColor(15, 55, 15);
        } else if (ledIndex < GREEN_LEDS + ORANGE_LEDS) {
            color = lit ? QColor(255, 160, 0) : QColor(60, 38, 0);
        } else {
            color = lit ? QColor(220, 40, 40) : QColor(55, 10, 10);
        }

        p.setBrush(color);
        p.setPen(Qt::NoPen);
        p.drawEllipse(MARGIN, y, LED_DIAMETER, LED_DIAMETER);
    }

    // Label
    p.setPen(Qt::white);
    QFont f = p.font();
    f.setPointSize(8);
    p.setFont(f);
    const int labelY = MARGIN + NUM_LEDS * (LED_DIAMETER + LED_GAP);
    p.drawText(QRect(0, labelY, width(), LABEL_HEIGHT), Qt::AlignCenter, label_);
}
