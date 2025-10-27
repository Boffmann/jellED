# jellED

jellED is built with a modular architecture consisting of several key components:

## jellED Core Libraries

The core libraries are platform independent libraries that provide core functionality for the jellED project.

### BeatDetection Library
Provides real-time filter chain for beat detection on sampled music signals.

![Beat Detection Chain](../../documentation/img/beatDetection.png)

**Components:**
- **Bandpass Filter**: Frequency-domain audio filtering
- **Envelope Detector**: Amplitude envelope extraction
- **Peak Detection**: Beat onset detection algorithms
- **Ring Buffer**: Efficient circular buffer for audio data

### PatternEngine Library
Generates and manages LED lighting patterns with configurable parameters.

**Features:**
- **Multiple Pattern Support**: The PatternEngine allows to combine and synchronize multiple patterns
- **Configurable Color Palettes**: RGB color management
- **Beat Synchronization**: Pattern timing based on detected beats

## Run and Debug Core Tests

To run and debug tests, first compile the package with debug information:

```
$ cd <path to build dir>
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
$ make
```

Afterwards, execute the tests using `ctest`.
To run a specific test, use the `-R` option:

```
$ ctest -R <test_name>
```

Example:

```
$ ctest -R BandpassFilterTest.TestApplyBandpass
```

## Platform Implementations

### ESP32 Implementation (`esp/`)
- **Hardware**: AZ-Delivery ESP32 DevKit V4
- **LED Output**: WS2812 addressable LED strips
- **Build System**: PlatformIO

### Raspberry Pi Implementation (`raspi/`)
- **Audio Input**: USB microphone support
- **File Processing**: WAV file analysis capabilities
- **Cross-platform**: Linux-compatible audio processing
- **Build System**: CMake

## Build and Debug

### Prerequisites
- **μC-Libs**: Clone https://github.com/Boffmann/mC_Libs
- **For ESP32**: PlatformIO IDE or CLI
- **For Raspberry Pi**: CMake, ALSA development libraries, and [libsoundio](http://libsound.io/#releases)

Execute the following commands to install ALSA dev libs and libsoundio.
Be sure that ALSA is listed as available during libsoundio installation.

```
sudo apt install libasound2-dev libpulse-dev
git clone https://github.com/andrewrk/libsoundio.git
cd libsoundio
cmake .
make
sudo make install
```



Make sure to specify the `MC_LIB_PATH` environment variable pointing to the `μC-Libs`'s root.
```
export MC_LIB_PATH=<path-to-μC-Libs>
```

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

1. **Setup the RaspberryPi:**
   ```bash
   chmod +x setupRaspi.sh
   ./setupRaspi.sh
   ```
   Activate Serial Connections with
   ```
    sudo raspi-config
    Select option 5, Interfacing options,
    then option P6, Serial,
    Select No to Serial Console
    Select Yes to Serial Port.
    Reboot.
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
