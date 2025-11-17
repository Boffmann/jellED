#include "AudioDisplay.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QGroupBox>
#include <QPainter>
#include <QPainterPath>
#include <iostream>

static constexpr int SIGNAL_DOWNSAMPLE_RATIO = 4;
static constexpr int ENVELOPE_DOWNSAMPLE_RATIO = 2;
static constexpr double DOWNSAMPLE_CUTOFF_FREQUENCY = 0.5;

static constexpr double AUTOMATIC_GAIN_CONTROL_TARGET_LEVEL = 0.4;

static constexpr double PEAK_DETECTION_ABSOLUTE_MIN_THRESHOLD = 0.05;
static constexpr double PEAK_DETECTION_THRESHOLD_REL = 0.1;
static constexpr double PEAK_DETECTION_MIN_PEAK_DISTANCE = 0.4;
static constexpr double PEAK_DETECTION_MAX_BPM = 180.0;

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

class BeatIndicatorWidget : public QWidget {
public:
    explicit BeatIndicatorWidget(int sampleRate, QWidget *parent = nullptr)
        : QWidget(parent),
        indication_cycles_total_(sampleRate * BEAT_INDICATION_TIME_MS / 1000),
        m_color_off(128,128,128),
        m_color_beat(255,0,0)
    {
        setFixedSize(100,100);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        setAutoFillBackground(false);
    }

    void setBeat(bool beat) {
        if (beat == m_beat) {
            return;
        }
        std::lock_guard<std::mutex> lock(dataMutex_);
        if (beat) {
            m_beat = true;
            beat_cycles_remaining = indication_cycles_total_;
            update();
        } else {
            beat_cycles_remaining--;
            if (beat_cycles_remaining <= 0) {
                m_beat = false;
                update();
            }
        }
    }

protected:

    void paintEvent(QPaintEvent * /*ev*/) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        qreal penW = 3.0;
        QRectF r = rect().adjusted(penW/2, penW/2, -penW/2, -penW/2);

        QPen pen(Qt::black, penW);
        p.setPen(pen);
        if (m_beat) {
            p.setBrush(m_color_beat);
        } else {
            p.setBrush(m_color_off);
        }
        p.drawEllipse(r);
    }

private:
    constexpr static int BEAT_INDICATION_TIME_MS = 50;
    bool m_beat;
    std::mutex dataMutex_;
    int beat_cycles_remaining;
    int indication_cycles_total_;
    QColor m_color_off;
    QColor m_color_beat;
};

AudioDisplay::AudioDisplay(std::string microphone_device_id, int displaySeconds, int refreshRate, QWidget* parent)
    : QMainWindow(parent)
    , sampleRate_(0)
    , usbMicro_(new jellED::UsbMicro(microphone_device_id, SoundIoBackendCoreAudio))
    , displaySeconds_(displaySeconds)
    , refreshRate_(refreshRate)
    , currentSamplesReceived_(0)
    , totalSamplesReceived_(0) {
    setWindowTitle("jellED - Oscilloscope");
    resize(1000, 600);

    usbMicro_->initialize();

    jellED::UsbMicro::print_available_input_devices(SoundIoBackendCoreAudio);

    this->sampleRate_ = usbMicro_->getSampleRate() / SIGNAL_DOWNSAMPLE_RATIO;
    std::cout << "Sample rate: " << sampleRate_ << std::endl;

    processorThread_ = new WaveformProcessor(sampleRate_, this);
    connect(processorThread_, &WaveformProcessor::displayDataReady, 
            this, &AudioDisplay::updateDisplay);
    processorThread_->start();
    setupUi();
}

AudioDisplay::~AudioDisplay() {
    processorThread_->stop();
    processorThread_->wait();

    // Stop the generator thread
    this->beatDetectionProcessor_->stop();
    this->beatDetectionProcessor_->wait();
}

void AudioDisplay::setupUi() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Add main sections
    mainLayout->addWidget(setupInfoPanel());
    
    QWidget* parameterizableWaveboxesWidget = new QWidget(this);
    QHBoxLayout* parameterizableWaveboxesLayout = new QHBoxLayout(parameterizableWaveboxesWidget);
    parameterizableWaveboxesLayout->addWidget(setupWaveformDisplays(), 1);
    parameterizableWaveboxesLayout->addWidget(setupParameterControls());
    
    mainLayout->addWidget(parameterizableWaveboxesWidget, 1);

    setupStatusBar();
}

QWidget* AudioDisplay::setupInfoPanel() {
    QGroupBox* infoGroup = new QGroupBox("Configuration", this);
    QHBoxLayout* infoLayout = new QHBoxLayout(infoGroup);

    QString infoText = QString("Sample Rate: %1 Hz | Display Window: %2 seconds | Refresh Rate: %3 FPS | Buffer Size: %4 samples")
        .arg(sampleRate_)
        .arg(displaySeconds_)
        .arg(refreshRate_)
        .arg(sampleRate_ * displaySeconds_);
    
    this->infoLabel_ = new QLabel(infoText, this);
    infoLayout->addWidget(this->infoLabel_);
    infoLayout->addStretch();

    clearButton_ = new QPushButton("Clear", this);
    clearButton_->setGeometry(QRect(10, 10, 100, 30));
    connect(clearButton_, &QPushButton::clicked, this, &AudioDisplay::onClearClicked);
    infoLayout->addWidget(clearButton_);
    
    return infoGroup;
}

QWidget* AudioDisplay::setupParameterControls() {
    QWidget* parameterWidget = new QWidget(this);
    QVBoxLayout* parameterLayout = new QVBoxLayout(parameterWidget);
    parameterLayout->setSpacing(0);
    parameterLayout->setContentsMargins(5, 5, 5, 5);

    // Envelope Downsample Rate control
    QGroupBox* envelopeDownsampleRateGroup = new QGroupBox("Envelope Downsample Rate", this);
    QHBoxLayout* envelopeDownsampleRateSliderLayout = new QHBoxLayout(envelopeDownsampleRateGroup);
    envelopeDownsampleRateSliderLayout->setSpacing(5);
    envelopeDownsampleRateSliderLayout->setContentsMargins(5, 5, 5, 5);
    this->envelopeDownsampleRateSlider_ = new QSlider(Qt::Horizontal, this);
    this->envelopeDownsampleRateSlider_->setRange(0, 4);
    // Find the power of 2 position for SIGNAL_DOWNSAMPLE_RATIO (log2)
    int initialPosition = 0;
    int temp = ENVELOPE_DOWNSAMPLE_RATIO;
    while (temp > 1) {
        temp >>= 1;
        initialPosition++;
    }
    this->envelopeDownsampleRateSlider_->setValue(initialPosition);
    this->envelopeDownsampleRateSlider_->setSingleStep(1);
    this->envelopeDownsampleRateSlider_->setTickPosition(QSlider::TicksBelow);
    this->envelopeDownsampleRateSlider_->setTickInterval(1);
    this->envelopeDownsampleRateValueLabel_ = new QLabel(QString::number(ENVELOPE_DOWNSAMPLE_RATIO), this);
    this->envelopeDownsampleRateValueLabel_->setFixedWidth(30);
    envelopeDownsampleRateSliderLayout->addWidget(this->envelopeDownsampleRateSlider_);
    envelopeDownsampleRateSliderLayout->addWidget(this->envelopeDownsampleRateValueLabel_);
    
    connect(this->envelopeDownsampleRateSlider_, &QSlider::valueChanged, this, &AudioDisplay::onEnvelopeDownsampleRateSliderChanged);

    QPushButton* applyButton = new QPushButton("Apply", this);
    applyButton->setFixedWidth(100);
    connect(applyButton, &QPushButton::clicked, this, &AudioDisplay::onApplyButtonClicked);


    // Downsample Rate control
    QGroupBox* downsampleCutoffFrequencyGroup = new QGroupBox("Downsample Cutoff Frequency", this);
    QHBoxLayout* downsampleCutoffFrequencySliderLayout = new QHBoxLayout(downsampleCutoffFrequencyGroup);
    downsampleCutoffFrequencySliderLayout->setSpacing(5);
    downsampleCutoffFrequencySliderLayout->setContentsMargins(5, 5, 5, 5);
    this->downsampleCutoffFrequencySlider_ = new QSlider(Qt::Horizontal, this);
    // Slider range 0-4 represents powers of 2: 2^0=1, 2^1=2, 2^2=4, 2^3=8, 2^4=16
    this->downsampleCutoffFrequencySlider_->setRange(0, 10);
    // Find the power of 2 position for SIGNAL_DOWNSAMPLE_RATIO (log2)
    this->downsampleCutoffFrequencySlider_->setValue(DOWNSAMPLE_CUTOFF_FREQUENCY * 10);
    this->downsampleCutoffFrequencySlider_->setSingleStep(1);
    this->downsampleCutoffFrequencySlider_->setTickPosition(QSlider::TicksBelow);
    this->downsampleCutoffFrequencySlider_->setTickInterval(1);
    this->downsampleCutoffFrequencyValueLabel_ = new QLabel(QString::number(DOWNSAMPLE_CUTOFF_FREQUENCY), this);
    this->downsampleCutoffFrequencyValueLabel_->setFixedWidth(30);
    downsampleCutoffFrequencySliderLayout->addWidget(this->downsampleCutoffFrequencySlider_);
    downsampleCutoffFrequencySliderLayout->addWidget(this->downsampleCutoffFrequencyValueLabel_);
    
    connect(this->downsampleCutoffFrequencySlider_, &QSlider::valueChanged, this, &AudioDisplay::onDownsampleCutoffFrequencySliderChanged);

    // Automatic Gain Control Target Level control
    QGroupBox* automaticGainControlTargetLevelGroup = new QGroupBox("Automatic Gain Control Target Level", this);
    QHBoxLayout* automaticGainControlTargetLevelLayout = new QHBoxLayout(automaticGainControlTargetLevelGroup);
    automaticGainControlTargetLevelLayout->setSpacing(5);
    automaticGainControlTargetLevelLayout->setContentsMargins(5, 5, 5, 5);
    this->automaticGainControlTargetLevelTextField_ = new QLineEdit(QString::number(0.4), this);
    automaticGainControlTargetLevelLayout->addWidget(this->automaticGainControlTargetLevelTextField_);

    beatIndicatorWidget_ = new BeatIndicatorWidget(sampleRate_, this);
    
    parameterLayout->addWidget(envelopeDownsampleRateGroup);
    parameterLayout->addWidget(downsampleCutoffFrequencyGroup);
    parameterLayout->addWidget(automaticGainControlTargetLevelGroup);
    parameterLayout->addWidget(setupPeakDetectionControls());
    parameterLayout->addWidget(applyButton);
    parameterLayout->addWidget(beatIndicatorWidget_, 0, Qt::AlignCenter);
    parameterLayout->addStretch();
    
    return parameterWidget;
}

QWidget* AudioDisplay::setupPeakDetectionControls() {
    // QWidget* peakDetectionWidget = new QWidget(this);

    QGroupBox* peakDetectionGroup = new QGroupBox("Peak Detection Parameters", this);
    QVBoxLayout* peakDetectionLayout = new QVBoxLayout(peakDetectionGroup);
    peakDetectionLayout->setSpacing(0);
    peakDetectionLayout->setContentsMargins(5, 5, 5, 5);

    // Peak Detection Absolute Min Threshold control
    QGroupBox* peakDetectionAbsoluteMinThresholdGroup = new QGroupBox("Peak Detection Absolute Min Threshold", this);
    QHBoxLayout* peakDetectionAbsoluteMinThresholdLayout = new QHBoxLayout(peakDetectionAbsoluteMinThresholdGroup);
    peakDetectionAbsoluteMinThresholdLayout->setSpacing(5);
    peakDetectionAbsoluteMinThresholdLayout->setContentsMargins(5, 5, 5, 5);
    this->peakDetectionAbsoluteMinThresholdTextField_ = new QLineEdit(QString::number(PEAK_DETECTION_ABSOLUTE_MIN_THRESHOLD), this);
    peakDetectionAbsoluteMinThresholdLayout->addWidget(this->peakDetectionAbsoluteMinThresholdTextField_);

    // Peak Detection Threshold Rel control
    QGroupBox* peakDetectionThresholdRelGroup = new QGroupBox("Peak Detection Threshold Rel", this);
    QHBoxLayout* peakDetectionThresholdRelLayout = new QHBoxLayout(peakDetectionThresholdRelGroup);
    peakDetectionThresholdRelLayout->setSpacing(5);
    peakDetectionThresholdRelLayout->setContentsMargins(5, 5, 5, 5);
    this->peakDetectionThresholdRelTextField_ = new QLineEdit(QString::number(PEAK_DETECTION_THRESHOLD_REL), this);
    peakDetectionThresholdRelLayout->addWidget(this->peakDetectionThresholdRelTextField_);

    // Peak Detection Min Peak Distance control
    QGroupBox* peakDetectionMinPeakDistanceGroup = new QGroupBox("Peak Detection Min Peak Distance", this);
    QHBoxLayout* peakDetectionMinPeakDistanceLayout = new QHBoxLayout(peakDetectionMinPeakDistanceGroup);
    peakDetectionMinPeakDistanceLayout->setSpacing(5);
    peakDetectionMinPeakDistanceLayout->setContentsMargins(5, 5, 5, 5);
    this->peakDetectionMinPeakDistanceTextField_ = new QLineEdit(QString::number(PEAK_DETECTION_MIN_PEAK_DISTANCE), this);
    peakDetectionMinPeakDistanceLayout->addWidget(this->peakDetectionMinPeakDistanceTextField_);

    // Peak Detection Max BPM control
    QGroupBox* peakDetectionMaxBpmGroup = new QGroupBox("Peak Detection Max BPM", this);
    QHBoxLayout* peakDetectionMaxBpmLayout = new QHBoxLayout(peakDetectionMaxBpmGroup);
    peakDetectionMaxBpmLayout->setSpacing(5);
    peakDetectionMaxBpmLayout->setContentsMargins(5, 5, 5, 5);
    this->peakDetectionMaxBpmTextField_ = new QLineEdit(QString::number(PEAK_DETECTION_MAX_BPM), this);
    peakDetectionMaxBpmLayout->addWidget(this->peakDetectionMaxBpmTextField_);

    peakDetectionLayout->addWidget(peakDetectionAbsoluteMinThresholdGroup);
    peakDetectionLayout->addWidget(peakDetectionThresholdRelGroup);
    peakDetectionLayout->addWidget(peakDetectionMinPeakDistanceGroup);
    peakDetectionLayout->addWidget(peakDetectionMaxBpmGroup);

    return peakDetectionGroup;
}

QWidget* AudioDisplay::setupWaveformDisplays() {
    QWidget* waveboxesWidget = new QWidget(this);
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
    
    return waveboxesWidget;
}

void AudioDisplay::setupStatusBar() {
    statusLabel_ = new QLabel(this);
    statusBar()->addWidget(statusLabel_);
}

void AudioDisplay::startBeatDetectionProcessor() {
    this->beatDetectionProcessor_ =
    new BeatDetectionProcessor(this,
        usbMicro_,
        SIGNAL_DOWNSAMPLE_RATIO,
        ENVELOPE_DOWNSAMPLE_RATIO,
        DOWNSAMPLE_CUTOFF_FREQUENCY,
        AUTOMATIC_GAIN_CONTROL_TARGET_LEVEL,
        PEAK_DETECTION_ABSOLUTE_MIN_THRESHOLD,
        PEAK_DETECTION_THRESHOLD_REL,
        PEAK_DETECTION_MIN_PEAK_DISTANCE,
        PEAK_DETECTION_MAX_BPM,
        this);
    this->beatDetectionProcessor_->start();
}

void AudioDisplay::addOriginalSample(const double sample) {
    originalSamplesWaveformWidget_->addSample(sample);
    totalSamplesReceived_++;
    currentSamplesReceived_++;
    beatIndicatorWidget_->setBeat(false); // Reset beat indicator at every sample
}

void AudioDisplay::addLowpassFilteredSample(const double sample) {
    lowpassFilteredWaveformWidget_->addSample(sample);
}

void AudioDisplay::addEnvelopeFilteredSample(const double sample) {
    envelopePeakWaveformWidget_->addSample(sample);
}

void AudioDisplay::addPeak() {
    envelopePeakWaveformWidget_->addPeak();
    beatIndicatorWidget_->setBeat(true);
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

void AudioDisplay::onEnvelopeDownsampleRateSliderChanged(int value) {
    // Convert slider position (0-4) to power of 2 (1, 2, 4, 8, 16)
    int envelopeDownsampleRate = 1 << value;  // Same as pow(2, value)
    this->envelopeDownsampleRateValueLabel_->setText(QString::number(envelopeDownsampleRate));
}

void AudioDisplay::onDownsampleCutoffFrequencySliderChanged(int value) {
    // Convert slider position (0-4) to power of 2 (1, 2, 4, 8, 16)
    double downsampleCutoffFrequency = 0.1 * value;
    this->downsampleCutoffFrequencyValueLabel_->setText(QString::number(downsampleCutoffFrequency));
}

void AudioDisplay::onApplyButtonClicked() {
    // Stop and delete the old thread
    this->beatDetectionProcessor_->stop();
    this->beatDetectionProcessor_->wait();
    delete this->beatDetectionProcessor_;

    int newEnvelopeDownsampleRatio = 1 << envelopeDownsampleRateSlider_->value();
    double newDownsampleCutoffFrequency = 0.1 * downsampleCutoffFrequencySlider_->value();

    double newAutomaticGainControlTargetLevel = automaticGainControlTargetLevelTextField_->text().toDouble();

    double newPeakDetectionAbsoluteMinThreshold = peakDetectionAbsoluteMinThresholdTextField_->text().toDouble();
    double newPeakDetectionThresholdRel = peakDetectionThresholdRelTextField_->text().toDouble();
    double newPeakDetectionMinPeakDistance = peakDetectionMinPeakDistanceTextField_->text().toDouble();
    double newPeakDetectionMaxBpm = peakDetectionMaxBpmTextField_->text().toDouble();

    // Update all waveform widgets with new sample rate
    originalSamplesWaveformWidget_->clearSamples();
    lowpassFilteredWaveformWidget_->clearSamples();
    envelopePeakWaveformWidget_->clearSamples();
    
    // Reset sample counters
    currentSamplesReceived_ = 0;

    // Create and start a new thread with the new parameters
    this->beatDetectionProcessor_ = new BeatDetectionProcessor(this,
        usbMicro_,
        SIGNAL_DOWNSAMPLE_RATIO,
        newEnvelopeDownsampleRatio,
        newDownsampleCutoffFrequency,
        newAutomaticGainControlTargetLevel,
        newPeakDetectionAbsoluteMinThreshold,
        newPeakDetectionThresholdRel,
        newPeakDetectionMinPeakDistance,
        newPeakDetectionMaxBpm,
        this);
    this->beatDetectionProcessor_->start();
    
    updateStatusBar();
}