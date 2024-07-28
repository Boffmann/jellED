#include "sound/wavFile.h"
#include "utils/raspiutils.h"
#include "sound/usbMicro.h"
#include "beatDetection/beatdetection.h"
#include <iostream>
#include <time.h>
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>
#include <fstream>

std::string microphone_device_id = "BuiltInMicrophoneDevice";

// Function to write audio samples to a file for debugging
void write_samples_to_file(const std::vector<float>& samples, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing" << std::endl;
        return;
    }
    
    // Write each sample on a separate line
    for (float sample : samples) {
        file << sample << std::endl;
    }
    
    file.close();
    std::cout << "Wrote " << samples.size() << " samples to " << filename << std::endl;
}

int main () {
    // Print all available input devices for debugging
    UsbMicro::print_available_input_devices();
    UsbMicro usbMicro(microphone_device_id);
    
    // Initialize the microphone before using it
    std::cout << "Initializing USB microphone..." << std::endl;
    usbMicro.initialize();
    
    AudioBuffer buffer;
    
    // Wait for first audio data with timeout (max 3 seconds)
    std::cout << "Waiting for first audio data..." << std::endl;
    bool got_data = false;
    auto start = std::chrono::steady_clock::now();
    const int timeout_seconds = 3;
    
    while (!got_data) {
        got_data = usbMicro.read(&buffer);
        if (got_data) {
            std::cout << "Audio data received! Starting main processing loop." << std::endl;
            break;
        }
        
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
        
        if (elapsed >= timeout_seconds) {
            std::cout << "Timeout reached (" << timeout_seconds << "s). Starting main loop anyway..." << std::endl;
            break;
        }
        
        // Sleep briefly to avoid busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    //RaspiPlatformUtils raspiUtils;
    //BeatDetector beatDetector(raspiUtils, buffer.samplingRate);
    std::cout << "Starting dedicated reader thread..." << std::endl;
    
    // For debugging: collect first 5 seconds of samples in a separate thread
    std::vector<float> debug_samples;
    const int samples_per_second = 48000; // Assuming 48kHz sample rate
    const int max_debug_samples = 5 * samples_per_second;
    std::atomic<bool> debug_complete(false);
    std::atomic<bool> debug_running(true);
    
    // Start debug collection thread
    std::thread debug_thread([&usbMicro, &debug_samples, max_debug_samples, &debug_complete, &debug_running]() {
        AudioBuffer debug_buffer;
        std::cout << "Debug thread started" << std::endl;
        
        while (debug_running && !debug_complete) {
            if (usbMicro.read(&debug_buffer)) {
                int samples_to_add = std::min((int)debug_buffer.num_samples, 
                                            max_debug_samples - (int)debug_samples.size());
                
                for (int i = 0; i < samples_to_add; i++) {
                    debug_samples.push_back(debug_buffer.buffer[i]);
                }
                
                if (debug_samples.size() >= max_debug_samples) {
                    debug_complete = true;
                    write_samples_to_file(debug_samples, "debug_audio_samples.txt");
                    std::cout << "Debug collection completed. File written." << std::endl;
                    break;
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    });
    
    // Start a dedicated reader thread that continuously reads from the ring buffer
    
    // Main thread waits for a while, then stops the reader
    std::cout << "Main thread: Running for 10 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // Stop the threads
    std::cout << "Main thread: Stopping threads..." << std::endl;
    debug_running = false;
    debug_thread.join();
    
    std::cout << "Program finished." << std::endl;
}
