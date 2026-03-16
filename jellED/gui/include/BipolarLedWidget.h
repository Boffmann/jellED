#ifndef __BIPOLARLEDWIDGET_H__
#define __BIPOLARLEDWIDGET_H__

#include <QColor>
#include <QString>
#include <QWidget>

// Horizontal LED row for bipolar signals in [-1, +1].
// 13 LEDs: 6 left of center (negative zone) + 1 center + 6 right (positive zone).
// Positive values fill rightward from center in positiveColor.
// Negative values fill leftward from center in negativeColor.
// The center LED is always lit as a neutral reference.
// Extreme labels are shown at each end to indicate what left/right mean.
class BipolarLedWidget : public QWidget {
    Q_OBJECT

public:
    // label       — shown on the far left (e.g. "TREND")
    // positiveColor / positiveLabel — right extreme (e.g. green / "BUILD")
    // negativeColor / negativeLabel — left extreme  (e.g. red   / "FADE")
    BipolarLedWidget(const QString& label,
                     const QColor&  positiveColor,
                     const QString& positiveLabel,
                     const QColor&  negativeColor,
                     const QString& negativeLabel,
                     QWidget* parent = nullptr);

    void setValue(double value);  // clamped to [-1, +1]

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    static constexpr int NUM_LEDS   = 13;
    static constexpr int CENTER_LED = 6;
    static constexpr int HALF_LEDS  = 6;

    QString label_;
    QColor  positiveColor_;
    QString positiveLabel_;
    QColor  negativeColor_;
    QString negativeLabel_;
    double  currentValue_;
};

#endif
