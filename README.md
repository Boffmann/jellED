# jellED - The Jellyfish with LED Support

A real-time audio-reactive LED lighting system that performs live beat detection and creates dynamic light shows. jellED combines advanced audio processing with sophisticated LED pattern generation to create an immersive visual experience synchronized with music.

## 🎵 Features

- **Real-time Beat Detection**: Advanced filter chain for live audio analysis
- **Dynamic LED Patterns**: Multiple pattern engines with configurable color palettes
- **Multi-Platform Support**: ESP32 and Raspberry Pi implementations
- **Bluetooth Configuration**: Remote pattern and color control via BLE
- **Audio Visualization Tools**: Python-based analysis and testing suite
- **Modular Architecture**: Reusable core libraries for different platforms

## 🏗️ Architecture

jellED is built with a modular architecture consisting of several key components:

### Core Libraries

#### BeatDetection Library
Provides real-time filter chain for beat detection on sampled music signals.

![Beat Detection Chain](documentation/img/beatDetection.png)

**Components:**
- **Bandpass Filter**: Frequency-domain audio filtering
- **Envelope Detector**: Amplitude envelope extraction
- **Peak Detection**: Beat onset detection algorithms
- **Ring Buffer**: Efficient circular buffer for audio data

#### PatternEngine Library
Generates and manages LED lighting patterns with configurable parameters.

**Features:**
- **Multiple Pattern Types**: Colored amplitude, alternating colors, and more
- **Configurable Color Palettes**: RGB color management
- **Beat Synchronization**: Pattern timing based on detected beats
- **JSON Configuration**: Dynamic pattern configuration via JSON

### Platform Implementations

#### ESP32 Implementation (`jellED/esp/`)
- **Hardware**: AZ-Delivery ESP32 DevKit V4
- **Audio Input**: INMP441 I2S MEMS microphone
- **LED Output**: WS2812 addressable LED strips
- **Communication**: Bluetooth Low Energy (BLE) for configuration
- **Build System**: PlatformIO

#### Raspberry Pi Implementation (`jellED/raspi/`)
- **Audio Input**: USB microphone support
- **File Processing**: WAV file analysis capabilities
- **Cross-platform**: Linux-compatible audio processing
- **Build System**: CMake

### Development Tools

#### Soundalyzer (`soundalyzer/`)
Python-based audio analysis and visualization tool for development and testing.

**Features:**
- Audio signal visualization
- Filter chain testing and validation
- Test file generation for C++ filter chain
- Real-time audio analysis

## 🚀 Quick Start

### Prerequisites

- **For ESP32**: PlatformIO IDE or CLI
- **For Raspberry Pi**: CMake, ALSA development libraries
- **For Development**: Python 3.7+, scipy, numpy, wave

### ESP32 Setup

1. **Clone the repository:**
   ```bash
   git clone https://github.com/Boffmann/jellED.git
   cd jellED
   ```

2. **Build and upload:**
   ```bash
   cd jellED/esp
   pio run -e az-delivery-devkit-v4 -t upload
   ```

3. **Monitor output:**
   ```bash
   pio run -e az-delivery-devkit-v4 -t monitor
   ```

### Raspberry Pi Setup

1. **Run the setup script:**
   ```bash
   chmod +x setupRaspi.sh
   ./setupRaspi.sh
   ```

2. **Build the application:**
   ```bash
   cd jellED/raspi
   mkdir build && cd build
   cmake ..
   make
   ```

3. **Run the application:**
   ```bash
   ./jellED
   ```

## 🔧 Hardware Configuration

### ESP32 Pin Configuration

| Component | Pin | Description |
|-----------|-----|-------------|
| LED Strip | 13 | WS2812 data line |
| LED Enable | 14 | LED power control |
| Microphone WS | 25 | I2S Word Select |
| Microphone SD | 32 | I2S Serial Data |
| Microphone SCK | 33 | I2S Serial Clock |
| Speaker Out | 26 | Audio output (optional) |

### Audio Hardware

#### INMP441 MEMS Microphone
- **Interface**: I2S digital output
- **Features**: Omnidirectional, DMA support
- **Sampling**: Configurable rate and bit depth
- **Documentation**: See [INMP441.md](documentation/INMP441.md)

#### WS2812 LED Strips
- **Protocol**: Custom timing-based protocol
- **Interface**: RMT (Remote Control Transceiver)
- **Color Format**: GRB (Green, Red, Blue)
- **Documentation**: See [WS2812.md](documentation/WS2812.md)

## 🎛️ Configuration

### Bluetooth Configuration

The ESP32 implementation supports Bluetooth Low Energy (BLE) for remote configuration:

- **Pattern Types**: Switch between different lighting patterns
- **Color Palettes**: Configure RGB color schemes
- **Timing Parameters**: Adjust beats per pattern and timing
- **Real-time Updates**: Dynamic configuration without restart

### Pattern Configuration

Patterns are configured via JSON format:

```json
{
  "pattern_engine": {
    "pattern1": "COLORED_AMPLITUDE",
    "pattern2": "ALTERNATING_COLORS",
    "beats_per_pattern": 4
  },
  "pattern": {
    "palette_color1": {"red": 255, "green": 0, "blue": 0},
    "palette_color2": {"red": 0, "green": 255, "blue": 0},
    "palette_color3": {"red": 0, "green": 0, "blue": 255}
  }
}
```

## 🧪 Testing and Development

### Running Tests

```bash
# Compile with debug information
cd jellED
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Run all tests
ctest

# Run specific test
ctest -R BandpassFilterTest.TestApplyBandpass
```

### Audio Analysis

Generate test files for the C++ filter chain:

```bash
cd soundalyzer
python soundalyzer.py --mode GEN_TEST_EXPECTATIONS
```

### ESP32 Audio Capture

Capture audio data from ESP32 for analysis:

```bash
pio run -e az-delivery-devkit-v4 -t monitor > soundinput.txt
```

Edit `soundinput.txt` to remove non-audio lines, then analyze with soundalyzer.

## 📁 Project Structure

```
jellED/
├── core/                    # Core libraries
│   ├── beatDetection/      # Audio processing library
│   └── patternEngine/      # LED pattern generation
├── esp/                    # ESP32 implementation
│   ├── src/               # ESP32 source code
│   └── platformio.ini     # PlatformIO configuration
├── raspi/                  # Raspberry Pi implementation
│   └── src/               # Raspberry Pi source code
├── soundalyzer/           # Python audio analysis tools
├── design/                # Hardware designs (KiCad)
├── documentation/         # Technical documentation
└── resources/            # Development resources
```

## 🔍 Dependencies

### Core Dependencies
- **AudioFile**: Audio file processing library by adamstark
- **Arduino Framework**: ESP32 development framework
- **PlatformIO**: Cross-platform build system

### Python Dependencies
- **scipy**: Scientific computing
- **numpy**: Numerical computing
- **wave**: WAV file processing

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass
6. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 👨‍💻 Author

**Hendrik Tjabben** - [hendrik.tjabben@gmail.com](mailto:hendrik.tjabben@gmail.com)

## 🙏 Acknowledgments

- **AudioFile Library**: [adamstark/AudioFile](https://github.com/adamstark/AudioFile)
- **ESP32 Arduino Framework**: Espressif Systems
- **PlatformIO**: PlatformIO Community

---

For detailed technical documentation, see the [documentation/](documentation/) directory.
