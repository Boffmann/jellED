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

The project is a collection of development tools that sophisticate implementation, visualization and debugging of the Real-time Beat Detection and the actual jellED implementation found in `/jellED`.

### Development Tools

#### Soundalyzer (`soundalyzer/`)
Python-based audio analysis and visualization tool for development and testing.

**Features:**
- Audio signal visualization
- Filter chain testing and validation
- Test file generation for C++ filter chain
- Real-time audio analysis

## Quick Start

For the jellED project see [jellED Readme](./jellED/README.md).

### Prerequisites

- **For Development**: Python 3.7+, scipy, numpy, wave

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
