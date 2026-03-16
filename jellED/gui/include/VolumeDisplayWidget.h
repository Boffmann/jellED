#ifndef __VOLUMEDISPLAYWIDGET_H__
#define __VOLUMEDISPLAYWIDGET_H__

#include <QWidget>
#include <QString>

class VolumeDisplayWidget : public QWidget {
    Q_OBJECT

public:
    explicit VolumeDisplayWidget(const QString& label, QWidget* parent = nullptr);

    // Accepts a raw (un-normalized) value. The widget normalizes using a
    // per-session running max, so each instance auto-scales independently.
    void setValue(double rawValue);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    static constexpr int NUM_LEDS   = 12;
    static constexpr int GREEN_LEDS = 6;   // LEDs 0–5
    static constexpr int ORANGE_LEDS = 3;  // LEDs 6–8
    // RED_LEDS = NUM_LEDS - GREEN_LEDS - ORANGE_LEDS = 3 (LEDs 9–11)

    QString label_;
    double  currentValue_;
    double  runningMax_;
};

#endif
