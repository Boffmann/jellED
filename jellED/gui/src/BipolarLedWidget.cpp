#include "BipolarLedWidget.h"

#include <QPainter>
#include <algorithm>
#include <cmath>

static constexpr int LED_DIAMETER    = 14;
static constexpr int LED_GAP         = 3;
static constexpr int V_MARGIN        = 5;
static constexpr int H_MARGIN        = 4;
static constexpr int LABEL_WIDTH     = 46;
static constexpr int EXTREME_WIDTH   = 38;  // width reserved for each extreme label
static constexpr int N_LEDS          = 13;  // must match BipolarLedWidget::NUM_LEDS

static constexpr int LED_AREA_WIDTH  = N_LEDS * (LED_DIAMETER + LED_GAP) - LED_GAP;
static constexpr int WIDGET_WIDTH    = LABEL_WIDTH + EXTREME_WIDTH
                                       + LED_AREA_WIDTH + EXTREME_WIDTH + H_MARGIN;
static constexpr int WIDGET_HEIGHT   = V_MARGIN * 2 + LED_DIAMETER;

static QColor dimmed(const QColor& c) {
    return QColor(c.red() / 7, c.green() / 7, c.blue() / 7);
}

BipolarLedWidget::BipolarLedWidget(const QString& label,
                                   const QColor&  positiveColor,
                                   const QString& positiveLabel,
                                   const QColor&  negativeColor,
                                   const QString& negativeLabel,
                                   QWidget* parent)
    : QWidget(parent)
    , label_(label)
    , positiveColor_(positiveColor)
    , positiveLabel_(positiveLabel)
    , negativeColor_(negativeColor)
    , negativeLabel_(negativeLabel)
    , currentValue_(0.0)
{
    setFixedSize(WIDGET_WIDTH, WIDGET_HEIGHT);
}

void BipolarLedWidget::setValue(double value) {
    currentValue_ = std::max(-1.0, std::min(1.0, value));
    update();
}

void BipolarLedWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor(20, 20, 20));

    QFont f = p.font();
    f.setPointSize(8);
    p.setFont(f);

    // Main label (left)
    p.setPen(Qt::white);
    p.drawText(QRect(0, 0, LABEL_WIDTH, WIDGET_HEIGHT), Qt::AlignCenter, label_);

    // Left extreme label — negativeLabel, colored
    p.setPen(negativeColor_);
    p.drawText(QRect(LABEL_WIDTH, 0, EXTREME_WIDTH, WIDGET_HEIGHT),
               Qt::AlignCenter, negativeLabel_);

    // Right extreme label — positiveLabel, colored
    const int rightLabelX = LABEL_WIDTH + EXTREME_WIDTH + LED_AREA_WIDTH;
    p.setPen(positiveColor_);
    p.drawText(QRect(rightLabelX, 0, EXTREME_WIDTH + H_MARGIN, WIDGET_HEIGHT),
               Qt::AlignCenter, positiveLabel_);

    // LEDs
    const int numLit  = static_cast<int>(std::abs(currentValue_) * HALF_LEDS);
    const bool positive = currentValue_ >= 0.0;
    const int ledsOriginX = LABEL_WIDTH + EXTREME_WIDTH;

    for (int i = 0; i < NUM_LEDS; ++i) {
        const int x = ledsOriginX + i * (LED_DIAMETER + LED_GAP);

        QColor color;
        if (i == CENTER_LED) {
            color = QColor(90, 90, 90);
        } else if (i > CENTER_LED) {
            const int dist = i - CENTER_LED;
            color = (positive && dist <= numLit) ? positiveColor_ : dimmed(positiveColor_);
        } else {
            const int dist = CENTER_LED - i;
            color = (!positive && dist <= numLit) ? negativeColor_ : dimmed(negativeColor_);
        }

        p.setBrush(color);
        p.setPen(Qt::NoPen);
        p.drawEllipse(x, V_MARGIN, LED_DIAMETER, LED_DIAMETER);
    }
}
