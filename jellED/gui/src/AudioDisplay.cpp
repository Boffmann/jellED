#include "AudioDisplay.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QGroupBox>
#include <QPainter>
#include <iostream>

static constexpr int SIGNAL_DOWNSAMPLE_RATIO = 4;
static constexpr int ENVELOPE_DOWNSAMPLE_RATIO = 8;

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

AudioDisplay::AudioDisplay(std::string microphone_device_id, int displaySeconds, int refreshRate, QWidget* parent)
    : QMainWindow(parent)
    , displaySeconds_(displaySeconds)
    , refreshRate_(refreshRate)
    , usbMicro_(new jellED::UsbMicro(microphone_device_id, SoundIoBackendCoreAudio))
    , currentSamplesReceived_(0)
    , totalSamplesReceived_(0)
    , sampleRate_(0) {
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

    // Downsample Rate control
    QGroupBox* downsampleRateGroup = new QGroupBox("Downsample Rate", this);
    QHBoxLayout* downsampleRateSliderLayout = new QHBoxLayout(downsampleRateGroup);
    downsampleRateSliderLayout->setSpacing(5);
    downsampleRateSliderLayout->setContentsMargins(5, 5, 5, 5);
    this->downsampleRateSlider_ = new QSlider(Qt::Horizontal, this);
    // Slider range 0-4 represents powers of 2: 2^0=1, 2^1=2, 2^2=4, 2^3=8, 2^4=16
    this->downsampleRateSlider_->setRange(0, 4);
    // Find the power of 2 position for SIGNAL_DOWNSAMPLE_RATIO (log2)
    int initialPosition = 0;
    int temp = SIGNAL_DOWNSAMPLE_RATIO;
    while (temp > 1) {
        temp >>= 1;
        initialPosition++;
    }
    this->downsampleRateSlider_->setValue(initialPosition);
    this->downsampleRateSlider_->setSingleStep(1);
    this->downsampleRateSlider_->setTickPosition(QSlider::TicksBelow);
    this->downsampleRateSlider_->setTickInterval(1);
    this->downsampleRateValueLabel_ = new QLabel(QString::number(SIGNAL_DOWNSAMPLE_RATIO), this);
    this->downsampleRateValueLabel_->setFixedWidth(30);
    downsampleRateSliderLayout->addWidget(this->downsampleRateSlider_);
    downsampleRateSliderLayout->addWidget(this->downsampleRateValueLabel_);
    
    connect(this->downsampleRateSlider_, &QSlider::valueChanged, this, &AudioDisplay::onDownsampleRateSliderChanged);

    // Envelope Downsample Rate control
    QGroupBox* envelopeDownsampleRateGroup = new QGroupBox("Envelope Downsample Rate", this);
    QHBoxLayout* envelopeDownsampleRateSliderLayout = new QHBoxLayout(envelopeDownsampleRateGroup);
    envelopeDownsampleRateSliderLayout->setSpacing(5);
    envelopeDownsampleRateSliderLayout->setContentsMargins(5, 5, 5, 5);
    this->envelopeDownsampleRateSlider_ = new QSlider(Qt::Horizontal, this);
    this->envelopeDownsampleRateSlider_->setRange(0, 4);
    // Find the power of 2 position for SIGNAL_DOWNSAMPLE_RATIO (log2)
    initialPosition = 0;
    temp = ENVELOPE_DOWNSAMPLE_RATIO;
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
    
    parameterLayout->addWidget(downsampleRateGroup);
    parameterLayout->addWidget(envelopeDownsampleRateGroup);
    parameterLayout->addWidget(applyButton);
    parameterLayout->addStretch();
    
    return parameterWidget;
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
    this->beatDetectionProcessor_ = new BeatDetectionProcessor(this, usbMicro_, SIGNAL_DOWNSAMPLE_RATIO, ENVELOPE_DOWNSAMPLE_RATIO, this);
    this->beatDetectionProcessor_->start();
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

void AudioDisplay::onDownsampleRateSliderChanged(int value) {
    // Convert slider position (0-4) to power of 2 (1, 2, 4, 8, 16)
    int downsampleRate = 1 << value;  // Same as pow(2, value)
    this->downsampleRateValueLabel_->setText(QString::number(downsampleRate));
}

void AudioDisplay::onEnvelopeDownsampleRateSliderChanged(int value) {
    // Convert slider position (0-4) to power of 2 (1, 2, 4, 8, 16)
    int envelopeDownsampleRate = 1 << value;  // Same as pow(2, value)
    this->envelopeDownsampleRateValueLabel_->setText(QString::number(envelopeDownsampleRate));
}

void AudioDisplay::onApplyButtonClicked() {
    // Stop and delete the old thread
    this->beatDetectionProcessor_->stop();
    this->beatDetectionProcessor_->wait();
    delete this->beatDetectionProcessor_;
    
    // Update sample rate - convert slider position to power of 2
    int newDownsampleRatio = 1 << downsampleRateSlider_->value();  // 2^value
    this->sampleRate_ = usbMicro_->getSampleRate() / newDownsampleRatio;

    int newEnvelopeDownsampleRatio = 1 << envelopeDownsampleRateSlider_->value();

    // Update all waveform widgets with new sample rate
    originalSamplesWaveformWidget_->updateSampleRate(sampleRate_);
    lowpassFilteredWaveformWidget_->updateSampleRate(sampleRate_);
    envelopePeakWaveformWidget_->updateSampleRate(sampleRate_);
    
    // Reset sample counters
    currentSamplesReceived_ = 0;

    // Update info panel
    QString infoText = QString("Sample Rate: %1 Hz | Display Window: %2 seconds | Refresh Rate: %3 FPS | Buffer Size: %4 samples")
        .arg(sampleRate_)
        .arg(displaySeconds_)
        .arg(refreshRate_)
        .arg(sampleRate_ * displaySeconds_);
    
    this->infoLabel_->setText(infoText);

    // Create and start a new thread with the new parameters
    this->beatDetectionProcessor_ = new BeatDetectionProcessor(this, usbMicro_, newDownsampleRatio, newEnvelopeDownsampleRatio, this);
    this->beatDetectionProcessor_->start();
    
    updateStatusBar();
}