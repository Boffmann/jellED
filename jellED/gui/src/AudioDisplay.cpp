#include "AudioDisplay.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QGroupBox>
#include <QPainter>
#include <iostream>

class VerticalLabel : public QLabel
{
public:
    explicit VerticalLabel(const QString &text, QWidget *parent = nullptr)
        : QLabel(text, parent) {
            setStyleSheet("background-color: lightgray; border: 1px solid gray;");
        }

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // Rotate coordinate system
        painter.rotate(-90);

        // Draw text rotated
        painter.drawText(0, 0, -height(), width(), Qt::AlignCenter, text());
    }

    QSize sizeHint() const override {
        QSize s = QLabel::sizeHint();
        return QSize(s.height(), s.width()); // swap width/height
    }
};

AudioDisplay::AudioDisplay(int sampleRate, int displaySeconds, int refreshRate, QWidget* parent)
    : QMainWindow(parent)
    , sampleRate_(sampleRate)
    , displaySeconds_(displaySeconds)
    , refreshRate_(refreshRate)
    , currentSamplesReceived_(0)
    , totalSamplesReceived_(0) {
    setWindowTitle("jellED - Oscilloscope");
    resize(1000, 600);
    processorThread_ = new WaveformProcessor(sampleRate_, this);
    connect(processorThread_, &WaveformProcessor::displayDataReady, 
            this, &AudioDisplay::updateDisplay);
    processorThread_->start();
    setupUi();
}

AudioDisplay::~AudioDisplay() {
    processorThread_->stop();
    processorThread_->wait();
}

void AudioDisplay::setupUi() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Info panel
    QGroupBox* infoGroup = new QGroupBox("Configuration", this);
    QHBoxLayout* infoLayout = new QHBoxLayout(infoGroup);

    QString infoText = QString("Sample Rate: %1 Hz | Display Window: %2 seconds | Refresh Rate: %3 FPS | Buffer Size: %4 samples")
        .arg(sampleRate_)
        .arg(displaySeconds_)
        .arg(refreshRate_)
        .arg(sampleRate_ * displaySeconds_);
    
    QLabel* infoLabel = new QLabel(infoText, this);
    infoLayout->addWidget(infoLabel);
    
    infoLayout->addStretch();

    clearButton_ = new QPushButton("Clear", this);
    clearButton_->setGeometry(QRect(10, 10, 100, 30));
    connect(clearButton_, &QPushButton::clicked, this, &AudioDisplay::onClearClicked);
    infoLayout->addWidget(clearButton_);
    
    mainLayout->addWidget(infoGroup);

    // Waveform display
    //QGroupBox* waveformGroup = new QGroupBox("Waveform Display", this);
    QWidget *waveboxesWidget = new QWidget(this);
    QVBoxLayout* waveformLayout = new QVBoxLayout(waveboxesWidget);

    QHBoxLayout* firstWaveformLayout = new QHBoxLayout();
    VerticalLabel* originalSamplesLabel = new VerticalLabel("Original Samples");
    originalSamplesLabel->setFixedWidth(30);
    originalSamplesWaveformWidget_ = new WaveformWidget(sampleRate_, displaySeconds_, this);
    firstWaveformLayout->addWidget(originalSamplesLabel);
    firstWaveformLayout->addWidget(originalSamplesWaveformWidget_, 1);

    QHBoxLayout* secondWaveformLayout = new QHBoxLayout();
    VerticalLabel* lowpassFilteredSamplesLabel = new VerticalLabel("Lowpass Filtered Samples");
    lowpassFilteredSamplesLabel->setFixedWidth(30);
    lowpassFilteredWaveformWidget_ = new WaveformWidget(sampleRate_, displaySeconds_, this);
    secondWaveformLayout->addWidget(lowpassFilteredSamplesLabel);
    secondWaveformLayout->addWidget(lowpassFilteredWaveformWidget_, 1);

    QHBoxLayout* thirdWaveformLayout = new QHBoxLayout();
    VerticalLabel* envelopeFilteredSamplesLabel = new VerticalLabel("Envelope Filtered Samples");
    envelopeFilteredSamplesLabel->setFixedWidth(30);
    envelopePeakWaveformWidget_ = new EnvelopePeakWidget(sampleRate_, displaySeconds_, this);
    thirdWaveformLayout->addWidget(envelopeFilteredSamplesLabel);
    thirdWaveformLayout->addWidget(envelopePeakWaveformWidget_, 1);

    waveformLayout->addLayout(firstWaveformLayout);
    waveformLayout->addLayout(secondWaveformLayout);
    waveformLayout->addLayout(thirdWaveformLayout);
    
    mainLayout->addWidget(waveboxesWidget, 1);

    // Status bar
    statusLabel_ = new QLabel(this);
    statusBar()->addWidget(statusLabel_);
}

void AudioDisplay::addOriginalSample(const double sample) {
    originalSamplesWaveformWidget_->addSample(sample);
    totalSamplesReceived_++;
    currentSamplesReceived_++;
}

void AudioDisplay::addLowpassFilteredSample(const double sample) {
    lowpassFilteredWaveformWidget_->addSample(sample);
}

void AudioDisplay::addEnvelopeFilteredSample(const double sample) {
    envelopePeakWaveformWidget_->addSample(sample);
}

void AudioDisplay::addPeak() {
    envelopePeakWaveformWidget_->addPeak();
}

void AudioDisplay::updateDisplay() {
    originalSamplesWaveformWidget_->updateWidget();
    lowpassFilteredWaveformWidget_->updateWidget();
    envelopePeakWaveformWidget_->updateWidget();
    updateStatusBar();  // Safe to update UI here - we're in the GUI thread
}

void AudioDisplay::updateStatusBar() {
    double currentSeconds = static_cast<double>(currentSamplesReceived_) / sampleRate_;
    double totalSeconds = static_cast<double>(totalSamplesReceived_) / sampleRate_;
    
    QString status = QString("Samples in buffer: %1 \t (%2 \t sec) | Total samples received: %3 \t (%4 \tsec)")
        .arg(currentSamplesReceived_)
        .arg(currentSeconds, 0, 'f', 2)
        .arg(totalSamplesReceived_)
        .arg(totalSeconds, 0, 'f', 2);
    
    statusLabel_->setText(status);
}

void AudioDisplay::onClearClicked() {
    originalSamplesWaveformWidget_->clearSamples();
    lowpassFilteredWaveformWidget_->clearSamples();
    envelopePeakWaveformWidget_->clearSamples();
    currentSamplesReceived_ = 0;
    updateStatusBar();
}
