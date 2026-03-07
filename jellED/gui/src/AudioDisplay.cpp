#include "AudioDisplay.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QGroupBox>
#include <QPainter>
#include <QPainterPath>
#include <iostream>

#include "sound/raspi/usbMicro.h"
#include "sound/raspi/waveStreamer.h"

static constexpr int SIGNAL_DOWNSAMPLE_RATIO = 4;
static constexpr int ENVELOPE_DOWNSAMPLE_RATIO = 1;
static constexpr double DOWNSAMPLE_CUTOFF_FREQUENCY = 0.5;

static constexpr double DEFAULT_NOVELTY_GAIN = 300.0;

static constexpr double AUTOMATIC_GAIN_CONTROL_TARGET_LEVEL = 0.4;

static constexpr double PEAK_DETECTION_ABSOLUTE_MIN_THRESHOLD = 0.05;
static constexpr double PEAK_DETECTION_THRESHOLD_REL = 0.1;
static constexpr double PEAK_DETECTION_MIN_PEAK_DISTANCE = 0.4;
static constexpr double PEAK_DETECTION_MAX_BPM = 250.0;

static const QString lableStyleSheet = "background-color: darkgray; border: 1px solid black; color: white;";

class VerticalLabel : public QLabel
{
public:
    explicit VerticalLabel(const QString &text, QWidget *parent = nullptr)
        : QLabel(text, parent) {
            setStyleSheet(lableStyleSheet);
        }

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter painter(this);

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
        setFixedSize(150,150);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        setAutoFillBackground(false);
    }

    void setBeat(bool beat) {
        std::lock_guard<std::mutex> lock(dataMutex_);
        if (beat) {
            m_beat = true;
            beat_cycles_remaining = indication_cycles_total_;
            update();
        } else {
            if (beat_cycles_remaining > 0) {
                beat_cycles_remaining--;
                if (beat_cycles_remaining <= 0) {
                    m_beat = false;
                    update();
                }
            }
        }
    }

protected:

    void paintEvent(QPaintEvent * /*ev*/) override {
        QPainter p(this);

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
    , updateRotation_(0)
    , sampleRate_(0)
    // , soundInput_(new jellED::UsbMicro(microphone_device_id, SoundIoBackendCoreAudio))
    , soundInput_(new jellED::WaveStreamer("/Users/tjabben/Documents/techno-drums-loop-120-bpm-1-44100.wav"))
    , displaySeconds_(displaySeconds)
    , refreshRate_(refreshRate)
    , currentSamplesReceived_(0)
    , totalSamplesReceived_(0)
    , currentDetectedBpm_(0.0) {
    setWindowTitle("jellED - Oscilloscope");
    resize(1200, 1000);

    soundInput_->initialize();

    jellED::UsbMicro::print_available_input_devices(SoundIoBackendCoreAudio);

    this->sampleRate_ = soundInput_->getSampleRate() / SIGNAL_DOWNSAMPLE_RATIO;
    std::cout << "Sample rate: " << sampleRate_ << std::endl;

    processorThread_ = new WaveformProcessor(sampleRate_, this);
    connect(processorThread_, &WaveformProcessor::displayDataReady, 
            this, &AudioDisplay::updateDisplay);
    processorThread_->start();
    setupUi();
}

AudioDisplay::~AudioDisplay() {
    // Stop and clean up processor thread
    processorThread_->stop();
    processorThread_->wait();
    delete processorThread_;

    // Stop and clean up beat detection processor thread
    beatDetectionProcessor_->stop();
    beatDetectionProcessor_->wait();
    delete beatDetectionProcessor_;

    // Clean up USB microphone
    delete soundInput_;
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
    
    startStopButton_ = new QPushButton("Pause", this);
    startStopButton_->setGeometry(QRect(10, 10, 100, 30));
    connect(startStopButton_, &QPushButton::clicked, this, &AudioDisplay::onStartStopClicked);
    infoLayout->addWidget(startStopButton_);
    
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
    this->automaticGainControlTargetLevelTextField_ = new QLineEdit(QString::number(AUTOMATIC_GAIN_CONTROL_TARGET_LEVEL), this);
    automaticGainControlTargetLevelLayout->addWidget(this->automaticGainControlTargetLevelTextField_);

    beatIndicatorWidget_ = new BeatIndicatorWidget(sampleRate_, this);

    // Novelty Gain control
    QGroupBox* noveltyGainGroup = new QGroupBox("Novelty Gain", this);
    QHBoxLayout* noveltyGainLayout = new QHBoxLayout(noveltyGainGroup);
    noveltyGainLayout->setSpacing(5);
    noveltyGainLayout->setContentsMargins(5, 5, 5, 5);
    this->noveltyGainTextField_ = new QLineEdit(QString::number(DEFAULT_NOVELTY_GAIN), this);
    noveltyGainLayout->addWidget(this->noveltyGainTextField_);
    
    parameterLayout->addWidget(envelopeDownsampleRateGroup);
    parameterLayout->addWidget(downsampleCutoffFrequencyGroup);
    parameterLayout->addWidget(noveltyGainGroup);
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
    QHBoxLayout* waveformLayoutsLayout = new QHBoxLayout(waveboxesWidget);
    QVBoxLayout* waveformLayoutLow = new QVBoxLayout();

    // Low frequency waveform layout
    QLabel* lowFrequencyLabel = new QLabel("Low Frequency 50 Hz - 150 Hz");
    lowFrequencyLabel->setAlignment(Qt::AlignCenter);
    lowFrequencyLabel->setFixedHeight(30);
    lowFrequencyLabel->setStyleSheet(lableStyleSheet);

    QHBoxLayout* firstWaveformLayoutLow = new QHBoxLayout();
    VerticalLabel* originalSamplesLabelLow = new VerticalLabel("Original Samples");
    originalSamplesLabelLow->setFixedWidth(30);
    originalSamplesWaveformWidgetLow_ = new WaveformWidget(sampleRate_, displaySeconds_, this);
    firstWaveformLayoutLow->addWidget(originalSamplesLabelLow);
    firstWaveformLayoutLow->addWidget(originalSamplesWaveformWidgetLow_, 1);

    QHBoxLayout* secondWaveformLayoutLow = new QHBoxLayout();
    VerticalLabel* lowpassFilteredSamplesLabel = new VerticalLabel("Lowpass Filtered Samples");
    lowpassFilteredSamplesLabel->setFixedWidth(30);
    lowpassFilteredWaveformWidgetLow_ = new WaveformWidget(sampleRate_, displaySeconds_, this);
    secondWaveformLayoutLow->addWidget(lowpassFilteredSamplesLabel);
    secondWaveformLayoutLow->addWidget(lowpassFilteredWaveformWidgetLow_, 1);

    QHBoxLayout* thirdWaveformLayoutLow = new QHBoxLayout();
    VerticalLabel* envelopeFilteredSamplesLabel = new VerticalLabel("Envelope Filtered Samples");
    envelopeFilteredSamplesLabel->setFixedWidth(30);
    envelopePeakWaveformWidgetLow_ = new EnvelopePeakWidget(sampleRate_, displaySeconds_, this);
    thirdWaveformLayoutLow->addWidget(envelopeFilteredSamplesLabel);
    thirdWaveformLayoutLow->addWidget(envelopePeakWaveformWidgetLow_, 1);

    waveformLayoutLow->addWidget(lowFrequencyLabel);
    waveformLayoutLow->addLayout(firstWaveformLayoutLow);
    waveformLayoutLow->addLayout(secondWaveformLayoutLow);
    waveformLayoutLow->addLayout(thirdWaveformLayoutLow);

    // Mid frequency waveform layout
    QLabel* midFrequencyLabel = new QLabel("Mid Frequency 150 Hz - 500 Hz");
    midFrequencyLabel->setAlignment(Qt::AlignCenter);
    midFrequencyLabel->setFixedHeight(30);
    midFrequencyLabel->setStyleSheet(lableStyleSheet);

    QVBoxLayout* waveformLayoutMid = new QVBoxLayout();
    QHBoxLayout* firstWaveformLayoutMid = new QHBoxLayout();
    VerticalLabel* originalSamplesLabelMid = new VerticalLabel("Original Samples");
    originalSamplesLabelMid->setFixedWidth(30);
    originalSamplesWaveformWidgetMid_ = new WaveformWidget(sampleRate_, displaySeconds_, this);
    firstWaveformLayoutMid->addWidget(originalSamplesLabelMid);
    firstWaveformLayoutMid->addWidget(originalSamplesWaveformWidgetMid_, 1);

    QHBoxLayout* secondWaveformLayoutMid = new QHBoxLayout();
    VerticalLabel* lowpassFilteredSamplesLabelMid = new VerticalLabel("Lowpass Filtered Samples");
    lowpassFilteredSamplesLabelMid->setFixedWidth(30);
    lowpassFilteredWaveformWidgetMid_ = new WaveformWidget(sampleRate_, displaySeconds_, this);
    secondWaveformLayoutMid->addWidget(lowpassFilteredSamplesLabelMid);
    secondWaveformLayoutMid->addWidget(lowpassFilteredWaveformWidgetMid_, 1);

    QHBoxLayout* thirdWaveformLayoutMid = new QHBoxLayout();
    VerticalLabel* envelopeFilteredSamplesLabelMid = new VerticalLabel("Envelope Filtered Samples");
    envelopeFilteredSamplesLabelMid->setFixedWidth(30);
    envelopePeakWaveformWidgetMid_ = new EnvelopePeakWidget(sampleRate_, displaySeconds_, this);
    thirdWaveformLayoutMid->addWidget(envelopeFilteredSamplesLabelMid);
    thirdWaveformLayoutMid->addWidget(envelopePeakWaveformWidgetMid_, 1);

    waveformLayoutMid->addWidget(midFrequencyLabel);
    waveformLayoutMid->addLayout(firstWaveformLayoutMid);
    waveformLayoutMid->addLayout(secondWaveformLayoutMid);
    waveformLayoutMid->addLayout(thirdWaveformLayoutMid);

    // High frequency waveform layout
    QLabel* highFrequencyLabel = new QLabel("High Frequency 2000 Hz - 5000 Hz");
    highFrequencyLabel->setAlignment(Qt::AlignCenter);
    highFrequencyLabel->setFixedHeight(30);
    highFrequencyLabel->setStyleSheet(lableStyleSheet);

    QVBoxLayout* waveformLayoutHigh = new QVBoxLayout();
    QHBoxLayout* firstWaveformLayoutHigh = new QHBoxLayout();
    VerticalLabel* originalSamplesLabelHigh = new VerticalLabel("Original Samples");
    originalSamplesLabelHigh->setFixedWidth(30);
    originalSamplesWaveformWidgetHigh_ = new WaveformWidget(sampleRate_, displaySeconds_, this);
    firstWaveformLayoutHigh->addWidget(originalSamplesLabelHigh);
    firstWaveformLayoutHigh->addWidget(originalSamplesWaveformWidgetHigh_, 1);

    QHBoxLayout* secondWaveformLayoutHigh = new QHBoxLayout();
    VerticalLabel* lowpassFilteredSamplesLabelHigh = new VerticalLabel("Lowpass Filtered Samples");
    lowpassFilteredSamplesLabelHigh->setFixedWidth(30);
    lowpassFilteredWaveformWidgetHigh_ = new WaveformWidget(sampleRate_, displaySeconds_, this);
    secondWaveformLayoutHigh->addWidget(lowpassFilteredSamplesLabelHigh);
    secondWaveformLayoutHigh->addWidget(lowpassFilteredWaveformWidgetHigh_, 1);

    QHBoxLayout* thirdWaveformLayoutHigh = new QHBoxLayout();
    VerticalLabel* envelopeFilteredSamplesLabelHigh = new VerticalLabel("Envelope Filtered Samples");
    envelopeFilteredSamplesLabelHigh->setFixedWidth(30);
    envelopePeakWaveformWidgetHigh_ = new EnvelopePeakWidget(sampleRate_, displaySeconds_, this);
    thirdWaveformLayoutHigh->addWidget(envelopeFilteredSamplesLabelHigh);
    thirdWaveformLayoutHigh->addWidget(envelopePeakWaveformWidgetHigh_, 1);

    waveformLayoutHigh->addWidget(highFrequencyLabel);
    waveformLayoutHigh->addLayout(firstWaveformLayoutHigh);
    waveformLayoutHigh->addLayout(secondWaveformLayoutHigh);
    waveformLayoutHigh->addLayout(thirdWaveformLayoutHigh);

    waveformLayoutsLayout->addLayout(waveformLayoutLow);
    waveformLayoutsLayout->addLayout(waveformLayoutMid);
    waveformLayoutsLayout->addLayout(waveformLayoutHigh);

    return waveboxesWidget;
}

void AudioDisplay::setupStatusBar() {
    statusLabel_ = new QLabel(this);
    QFont monoFont("Monaco");  // or "Monaco", "Consolas", "Monospace"
    monoFont.setPointSize(10);
    statusLabel_->setFont(monoFont);
    statusBar()->addWidget(statusLabel_);
}

void AudioDisplay::startBeatDetectionProcessor() {
    jellED::BeatDetector* beatDetector = jellED::BeatDetector::Builder(soundInput_->getSampleRate() / SIGNAL_DOWNSAMPLE_RATIO)
        .setEnvelopeDownsampleRatio(ENVELOPE_DOWNSAMPLE_RATIO)
        .setNoveltyGain(DEFAULT_NOVELTY_GAIN)
        .setPeakDetectionAbsoluteMinThreshold(PEAK_DETECTION_ABSOLUTE_MIN_THRESHOLD)
        .setPeakDetectionThresholdRel(PEAK_DETECTION_THRESHOLD_REL)
        .setPeakDetectionMinPeakDistance(PEAK_DETECTION_MIN_PEAK_DISTANCE)
        .setPeakDetectionMaxBpm(PEAK_DETECTION_MAX_BPM)
        .build();

    this->beatDetectionProcessor_ = new BeatDetectionProcessor(this, soundInput_, beatDetector, DOWNSAMPLE_CUTOFF_FREQUENCY, AUTOMATIC_GAIN_CONTROL_TARGET_LEVEL, SIGNAL_DOWNSAMPLE_RATIO, this);
    
    this->beatDetectionProcessor_->start();
}

void AudioDisplay::addOriginalSample(const double sample) {
    originalSamplesWaveformWidgetLow_->addSample(sample);
    originalSamplesWaveformWidgetMid_->addSample(sample);
    originalSamplesWaveformWidgetHigh_->addSample(sample);
    totalSamplesReceived_++;
    currentSamplesReceived_++;
    beatIndicatorWidget_->setBeat(false); // Reset beat indicator at every sample
}

void AudioDisplay::addLowpassFilteredSampleLow(const double sample) {
    lowpassFilteredWaveformWidgetLow_->addSample(sample);
}

void AudioDisplay::addLowpassFilteredSampleMid(const double sample) {
    lowpassFilteredWaveformWidgetMid_->addSample(sample);
}

void AudioDisplay::addLowpassFilteredSampleHigh(const double sample) {
    lowpassFilteredWaveformWidgetHigh_->addSample(sample);
}

void AudioDisplay::addEnvelopeFilteredSampleLow(const double sample) {
    envelopePeakWaveformWidgetLow_->addSample(sample);
}

void AudioDisplay::addEnvelopeFilteredSampleMid(const double sample) {
    envelopePeakWaveformWidgetMid_->addSample(sample);
}

void AudioDisplay::addEnvelopeFilteredSampleHigh(const double sample) {
    envelopePeakWaveformWidgetHigh_->addSample(sample);
}

void AudioDisplay::addPeakLow() {
    envelopePeakWaveformWidgetLow_->addPeak();
    // beatIndicatorWidget_->setBeat(true);
}

void AudioDisplay::addPeakMid() {
    envelopePeakWaveformWidgetMid_->addPeak();
    // beatIndicatorWidget_->setBeat(true);
}

void AudioDisplay::addPeakHigh() {
    envelopePeakWaveformWidgetHigh_->addPeak();
    // beatIndicatorWidget_->setBeat(true);
}

void AudioDisplay::addCombinedPeak() {
    beatIndicatorWidget_->setBeat(true);
}

void AudioDisplay::addCurrentDetectedBpm(const double bpm) {
    this->currentDetectedBpm_ = bpm;
}

void AudioDisplay::updateDisplay() {
    updateStatusBar();  // Safe to update UI here - we're in the GUI thread

    switch (updateRotation_) {
        case 0:
            originalSamplesWaveformWidgetLow_->updateWidget();
            lowpassFilteredWaveformWidgetLow_->updateWidget();
            envelopePeakWaveformWidgetLow_->updateWidget();
            break;
        case 1:
            originalSamplesWaveformWidgetMid_->updateWidget();
            lowpassFilteredWaveformWidgetMid_->updateWidget();
            envelopePeakWaveformWidgetMid_->updateWidget();
            break;
        case 2:
            originalSamplesWaveformWidgetHigh_->updateWidget();
            lowpassFilteredWaveformWidgetHigh_->updateWidget();
            envelopePeakWaveformWidgetHigh_->updateWidget();
            break;
    }
    
    updateRotation_ = (updateRotation_ + 1) % 3;
}

void AudioDisplay::updateStatusBar() {
    // Only update every 30 calls (~1 second at 30 FPS)
    static int updateCounter = 0;
    if (++updateCounter % 30 != 0) return;
    double currentSeconds = static_cast<double>(currentSamplesReceived_) / sampleRate_;
    double totalSeconds = static_cast<double>(totalSamplesReceived_) / sampleRate_;

    
    QString status = QString("Samples in buffer: %1 (%2 sec) | Total samples received: %3 (%4 sec) | Current detected BPM: %5")
        .arg(currentSamplesReceived_, 8)
        .arg(currentSeconds, 7, 'f', 2)
        .arg(totalSamplesReceived_, 8)
        .arg(totalSeconds, 7, 'f', 2)
        .arg(currentDetectedBpm_, 7, 'f', 2);  // Add width 7, format 'f', precision 2
    
    statusLabel_->setText(status);
}

void AudioDisplay::onClearClicked() {
    originalSamplesWaveformWidgetLow_->clearSamples();
    lowpassFilteredWaveformWidgetLow_->clearSamples();
    envelopePeakWaveformWidgetLow_->clearSamples();
    originalSamplesWaveformWidgetMid_->clearSamples();
    lowpassFilteredWaveformWidgetMid_->clearSamples();
    envelopePeakWaveformWidgetMid_->clearSamples();
    originalSamplesWaveformWidgetHigh_->clearSamples();
    lowpassFilteredWaveformWidgetHigh_->clearSamples();
    envelopePeakWaveformWidgetHigh_->clearSamples();
    currentSamplesReceived_ = 0;
    updateStatusBar();
}

void AudioDisplay::onStartStopClicked() {
    if (startStopButton_->text() == "Resume") {
        startStopButton_->setText("Pause");
        onApplyButtonClicked();
    } else {
        startStopButton_->setText("Resume");
        this->beatDetectionProcessor_->stop();
        this->beatDetectionProcessor_->wait();
    }
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
    double newNoveltyGain = noveltyGainTextField_->text().toDouble();

    double newAutomaticGainControlTargetLevel = automaticGainControlTargetLevelTextField_->text().toDouble();

    double newPeakDetectionAbsoluteMinThreshold = peakDetectionAbsoluteMinThresholdTextField_->text().toDouble();
    double newPeakDetectionThresholdRel = peakDetectionThresholdRelTextField_->text().toDouble();
    double newPeakDetectionMinPeakDistance = peakDetectionMinPeakDistanceTextField_->text().toDouble();
    double newPeakDetectionMaxBpm = peakDetectionMaxBpmTextField_->text().toDouble();

    // Update all waveform widgets with new sample rate
    originalSamplesWaveformWidgetLow_->clearSamples();
    lowpassFilteredWaveformWidgetLow_->clearSamples();
    envelopePeakWaveformWidgetLow_->clearSamples();
    originalSamplesWaveformWidgetMid_->clearSamples();
    lowpassFilteredWaveformWidgetMid_->clearSamples();
    envelopePeakWaveformWidgetMid_->clearSamples();
    originalSamplesWaveformWidgetHigh_->clearSamples();
    lowpassFilteredWaveformWidgetHigh_->clearSamples();
    envelopePeakWaveformWidgetHigh_->clearSamples();
    
    // Reset sample counters
    currentSamplesReceived_ = 0;

    // Create and start a new thread with the new parameters

    std::cout << "BeatDetectionProcessor constructed with signalDownsampleRatio: " << SIGNAL_DOWNSAMPLE_RATIO
    << ", envelopeDownsampleRatio: " << newEnvelopeDownsampleRatio
    << ", noveltyGain: " << newNoveltyGain
    << ", downsampleCutoffFrequency: " << newAutomaticGainControlTargetLevel
    << ", automaticGainControlTargetLevel: " << newAutomaticGainControlTargetLevel
    << ", peakDetectionAbsoluteMinThreshold: " << newPeakDetectionAbsoluteMinThreshold
    << ", peakDetectionThresholdRel: " << newPeakDetectionThresholdRel
    << ", peakDetectionMinPeakDistance: " << newPeakDetectionMinPeakDistance
    << ", peakDetectionMaxBpm: " << newPeakDetectionMaxBpm
    << std::endl;

    jellED::BeatDetector* beatDetector = jellED::BeatDetector::Builder(soundInput_->getSampleRate() / SIGNAL_DOWNSAMPLE_RATIO)
        .setEnvelopeDownsampleRatio(newEnvelopeDownsampleRatio)
        .setNoveltyGain(newNoveltyGain)
        .setPeakDetectionAbsoluteMinThreshold(newPeakDetectionAbsoluteMinThreshold)
        .setPeakDetectionThresholdRel(newPeakDetectionThresholdRel)
        .setPeakDetectionMinPeakDistance(newPeakDetectionMinPeakDistance)
        .setPeakDetectionMaxBpm(newPeakDetectionMaxBpm)
        .build();

    this->beatDetectionProcessor_ = new BeatDetectionProcessor(this, soundInput_, beatDetector, newDownsampleCutoffFrequency, newAutomaticGainControlTargetLevel, SIGNAL_DOWNSAMPLE_RATIO, this);
    
    this->beatDetectionProcessor_->start();
    
    updateStatusBar();
}