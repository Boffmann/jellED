#include "AudioDisplay.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QGroupBox>
#include <QPainter>
#include <QPainterPath>
#include <chrono>
#include <iostream>
#include <thread>

#include "sound/raspi/usbMicro.h"
#include "sound/raspi/waveStreamer.h"
#include "sound/raspi/wavePlayer.h"

static constexpr int SIGNAL_DOWNSAMPLE_RATIO = 4;
static const std::string DEFAULT_WAV_FILE = "/Users/tjabben/Documents/techno-drums-loop-120-bpm-1-44100.wav";

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
    , soundInput_(new jellED::WaveStreamer(DEFAULT_WAV_FILE))
    , currentInputMode_(AudioInputMode::WavFile)
    , currentSourcePath_(DEFAULT_WAV_FILE)
    , displaySeconds_(displaySeconds)
    , refreshRate_(refreshRate)
    , currentSamplesReceived_(0)
    , totalSamplesReceived_(0)
    , currentDetectedBpm_(0.0)
    , currentVolumeLow_(0.0)
    , currentVolumeMid_(0.0)
    , currentVolumeHigh_(0.0)
    , currentOverallVolume_(0.0)
    , currentVolumeTrend_(0.0)
    , currentSpectralTilt_(0.0) {
    setWindowTitle("jellED - Oscilloscope");
    resize(1200, 1000);

    soundInput_->initialize();

    this->sampleRate_ = soundInput_->getSampleRate() / SIGNAL_DOWNSAMPLE_RATIO;
    std::cout << "Sample rate: " << sampleRate_ << std::endl;

    processorThread_ = new WaveformProcessor(sampleRate_, this);
    connect(processorThread_, &WaveformProcessor::displayDataReady, 
            this, &AudioDisplay::updateDisplay);
    processorThread_->start();

    configuratorWindow_ = new ConfiguratorWindow(this);
    configuratorWindow_->setWavFilePath(QString::fromStdString(DEFAULT_WAV_FILE));
    connect(configuratorWindow_, &ConfiguratorWindow::applyRequested,
            this, &AudioDisplay::onApplyConfig);

    setupUi();
}

AudioDisplay::~AudioDisplay() {
    processorThread_->stop();
    processorThread_->wait();
    delete processorThread_;

    beatDetectionProcessor_->stop();
    beatDetectionProcessor_->wait();
    delete beatDetectionProcessor_;

    delete soundInput_;
}

void AudioDisplay::setupUi() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    mainLayout->addWidget(setupInfoPanel());
    
    QWidget* waveformsAndIndicatorWidget = new QWidget(this);
    QHBoxLayout* waveformsAndIndicatorLayout = new QHBoxLayout(waveformsAndIndicatorWidget);

    waveformsAndIndicatorLayout->addWidget(setupWaveformDisplays(), 1);

    beatIndicatorWidget_ = new BeatIndicatorWidget(sampleRate_, this);

    volumeLowWidget_     = new VolumeDisplayWidget("LOW",  this);
    volumeMidWidget_     = new VolumeDisplayWidget("MID",  this);
    volumeHighWidget_    = new VolumeDisplayWidget("HIGH", this);
    volumeOverallWidget_ = new VolumeDisplayWidget("ALL",  this);
    volumeTrendWidget_   = new BipolarLedWidget("TREND",
                               QColor(50, 220, 50),  "BUILD",
                               QColor(220, 60, 60),  "FADE",
                               this);
    spectralTiltWidget_  = new BipolarLedWidget("TILT",
                               QColor(255, 140, 0),  "BASS",
                               QColor(80, 160, 255), "TREB",
                               this);

    QHBoxLayout* metersRow = new QHBoxLayout();
    metersRow->setSpacing(4);
    metersRow->addWidget(volumeLowWidget_);
    metersRow->addWidget(volumeMidWidget_);
    metersRow->addWidget(volumeHighWidget_);
    metersRow->addWidget(volumeOverallWidget_);

    QVBoxLayout* sidePanel = new QVBoxLayout();
    sidePanel->addWidget(beatIndicatorWidget_, 0, Qt::AlignHCenter);
    sidePanel->addLayout(metersRow);
    sidePanel->addWidget(volumeTrendWidget_);
    sidePanel->addWidget(spectralTiltWidget_);
    sidePanel->addStretch();
    waveformsAndIndicatorLayout->addLayout(sidePanel);

    mainLayout->addWidget(waveformsAndIndicatorWidget, 1);

    setupStatusBar();
}

QWidget* AudioDisplay::setupInfoPanel() {
    QGroupBox* infoGroup = new QGroupBox(this);
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

    muteButton_ = new QPushButton("Mute", this);
    muteButton_->setGeometry(QRect(10, 10, 100, 30));
    muteButton_->setCheckable(true);
    connect(muteButton_, &QPushButton::clicked, this, &AudioDisplay::onMuteClicked);
    infoLayout->addWidget(muteButton_);

    configureButton_ = new QPushButton("Configure...", this);
    configureButton_->setGeometry(QRect(10, 10, 100, 30));
    connect(configureButton_, &QPushButton::clicked, this, &AudioDisplay::onConfigureClicked);
    infoLayout->addWidget(configureButton_);
    
    return infoGroup;
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
    QFont monoFont("Monaco");
    monoFont.setPointSize(10);
    statusLabel_->setFont(monoFont);
    statusBar()->addWidget(statusLabel_);
}

void AudioDisplay::startBeatDetectionProcessor() {
    const jellED::BeatDetectionConfig config = configuratorWindow_->currentConfig();
    this->beatDetectionProcessor_ = new BeatDetectionProcessor(this, soundInput_, config, SIGNAL_DOWNSAMPLE_RATIO, this);
    this->beatDetectionProcessor_->start();
}

void AudioDisplay::addOriginalSample(const double sample) {
    originalSamplesWaveformWidgetLow_->addSample(sample);
    originalSamplesWaveformWidgetMid_->addSample(sample);
    originalSamplesWaveformWidgetHigh_->addSample(sample);
    totalSamplesReceived_++;
    currentSamplesReceived_++;
    beatIndicatorWidget_->setBeat(false);
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
}

void AudioDisplay::addPeakMid() {
    envelopePeakWaveformWidgetMid_->addPeak();
}

void AudioDisplay::addPeakHigh() {
    envelopePeakWaveformWidgetHigh_->addPeak();
}

void AudioDisplay::setThresholdLow(double threshold) {
    envelopePeakWaveformWidgetLow_->setCurrentThreshold(threshold);
}

void AudioDisplay::setThresholdMid(double threshold) {
    envelopePeakWaveformWidgetMid_->setCurrentThreshold(threshold);
}

void AudioDisplay::setThresholdHigh(double threshold) {
    envelopePeakWaveformWidgetHigh_->setCurrentThreshold(threshold);
}

void AudioDisplay::addCombinedPeak() {
    beatIndicatorWidget_->setBeat(true);
}

void AudioDisplay::addCurrentDetectedBpm(const double bpm) {
    this->currentDetectedBpm_ = bpm;
}

void AudioDisplay::updateDisplay() {
    updateStatusBar();

    volumeLowWidget_->setValue(currentVolumeLow_.load());
    volumeMidWidget_->setValue(currentVolumeMid_.load());
    volumeHighWidget_->setValue(currentVolumeHigh_.load());
    volumeOverallWidget_->setValue(currentOverallVolume_.load());
    volumeTrendWidget_->setValue(currentVolumeTrend_.load());
    spectralTiltWidget_->setValue(currentSpectralTilt_.load());

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
    static int updateCounter = 0;
    if (++updateCounter % 30 != 0) return;
    double currentSeconds = static_cast<double>(currentSamplesReceived_) / sampleRate_;
    double totalSeconds = static_cast<double>(totalSamplesReceived_) / sampleRate_;

    
    QString status = QString("Samples in buffer: %1 (%2 sec) | Total samples received: %3 (%4 sec) | Current detected BPM: %5")
        .arg(currentSamplesReceived_, 8)
        .arg(currentSeconds, 7, 'f', 2)
        .arg(totalSamplesReceived_, 8)
        .arg(totalSeconds, 7, 'f', 2)
        .arg(currentDetectedBpm_, 7, 'f', 2);
    
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
        onApplyConfig(configuratorWindow_->currentConfig());
    } else {
        startStopButton_->setText("Resume");
        this->beatDetectionProcessor_->stop();
        this->beatDetectionProcessor_->wait();
    }
}

void AudioDisplay::onMuteClicked() {
    bool muted = muteButton_->isChecked();
    muteButton_->setText(muted ? "Unmute" : "Mute");
    jellED::WaveStreamer* streamer = dynamic_cast<jellED::WaveStreamer*>(soundInput_);
    if (streamer) {
        streamer->setMuted(muted);
    }
}

void AudioDisplay::onConfigureClicked() {
    configuratorWindow_->show();
    configuratorWindow_->raise();
    configuratorWindow_->activateWindow();
}

void AudioDisplay::setVolumeLow(double volume)    { currentVolumeLow_.store(volume); }
void AudioDisplay::setVolumeMid(double volume)    { currentVolumeMid_.store(volume); }
void AudioDisplay::setVolumeHigh(double volume)   { currentVolumeHigh_.store(volume); }
void AudioDisplay::setOverallVolume(double volume){ currentOverallVolume_.store(volume); }
void AudioDisplay::setVolumeTrend(double trend)   { currentVolumeTrend_.store(trend); }
void AudioDisplay::setSpectralTilt(double tilt)   { currentSpectralTilt_.store(tilt); }

void AudioDisplay::onApplyConfig(const jellED::BeatDetectionConfig& config) {
    this->beatDetectionProcessor_->stop();
    this->beatDetectionProcessor_->wait();
    delete this->beatDetectionProcessor_;

    AudioInputMode newMode = configuratorWindow_->currentAudioInputMode();
    std::string newSource = configuratorWindow_->currentAudioSource().toStdString();
    std::string outputDevice = configuratorWindow_->currentOutputDevice().toStdString();

    bool sourceChanged = (newMode != currentInputMode_ || newSource != currentSourcePath_);
    if (sourceChanged && !newSource.empty()) {
        delete soundInput_;
        soundInput_ = nullptr;

        bool initialized = false;
        static constexpr int MAX_INIT_RETRIES = 3;
        static constexpr int INIT_RETRY_BASE_DELAY_MS = 500;

        for (int attempt = 0; attempt < MAX_INIT_RETRIES && !initialized; ++attempt) {
            if (attempt > 0) {
                int delayMs = INIT_RETRY_BASE_DELAY_MS * attempt;
                std::cerr << "Retrying initialization (attempt " << (attempt + 1)
                          << ", waiting " << delayMs << "ms)..." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
            }

            if (newMode == AudioInputMode::Microphone) {
                soundInput_ = new jellED::UsbMicro(newSource, SoundIoBackendCoreAudio);
            } else {
                soundInput_ = new jellED::WaveStreamer(newSource, outputDevice);
            }

            if (soundInput_->initialize()) {
                initialized = true;
            } else {
                delete soundInput_;
                soundInput_ = nullptr;
            }
        }

        if (!initialized) {
            std::cerr << "Failed to initialize " << newSource
                      << " after " << MAX_INIT_RETRIES << " attempts, falling back to default WAV file" << std::endl;
            soundInput_ = new jellED::WaveStreamer(DEFAULT_WAV_FILE, outputDevice);
            soundInput_->initialize();
            newMode = AudioInputMode::WavFile;
            newSource = DEFAULT_WAV_FILE;
            statusLabel_->setText("Error: failed to initialize audio source, using default");
        }

        currentInputMode_ = newMode;
        currentSourcePath_ = newSource;
        sampleRate_ = soundInput_->getSampleRate() / SIGNAL_DOWNSAMPLE_RATIO;

        // Reset mute button: mute only applies to WavFile playback
        muteButton_->setChecked(false);
        muteButton_->setText("Mute");
        muteButton_->setEnabled(newMode == AudioInputMode::WavFile);

        QString infoText = QString("Sample Rate: %1 Hz | Display Window: %2 seconds | Refresh Rate: %3 FPS | Buffer Size: %4 samples")
            .arg(sampleRate_).arg(displaySeconds_).arg(refreshRate_).arg(sampleRate_ * displaySeconds_);
        infoLabel_->setText(infoText);
    }

    onClearClicked();

    this->beatDetectionProcessor_ = new BeatDetectionProcessor(this, soundInput_, config, SIGNAL_DOWNSAMPLE_RATIO, this);
    this->beatDetectionProcessor_->start();
    
    updateStatusBar();
}
