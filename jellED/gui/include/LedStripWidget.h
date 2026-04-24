#ifndef __LED_STRIP_WIDGET_H__
#define __LED_STRIP_WIDGET_H__

#include <QTimer>
#include <QWidget>
#include <mutex>
#include <vector>

#include "pattern_colors.h"

// Renders the current pattern frame as N circles left-to-right.
// Writes (setColors) come from the audio thread; repaints happen on the GUI
// thread via a QTimer. A short mutex guards the pending/displayed buffers —
// never hold it across any Qt widget call.
class LedStripWidget : public QWidget {
    Q_OBJECT

public:
    explicit LedStripWidget(int numLeds, QWidget* parent = nullptr);

    // Thread-safe. Copies up to numLeds colors under a short mutex and flags
    // the widget as dirty; the next timer tick picks up the new frame.
    void setColors(const jellED::pattern_color* colors, int count);

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onRefreshTimer();

private:
    const int numLeds_;
    std::vector<jellED::pattern_color> pending_;
    std::vector<jellED::pattern_color> displayed_;
    std::mutex mutex_;
    bool pendingDirty_;
    QTimer refreshTimer_;
};

#endif
