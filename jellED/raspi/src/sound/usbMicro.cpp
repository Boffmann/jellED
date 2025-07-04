#include "sound/usbMicro.h"

#include <stdio.h>
#include <string.h>

#include <stdint.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>

#define NO_SUPPORTED_SAMPLE_RATE -1

// https://github.com/andrewrk/libsoundio/blob/master/example/sio_microphone.c
UsbMicro::UsbMicro(std::string& device_id)
    :   soundio{nullptr},
        microphone{nullptr},
        mic_in_stream{nullptr},
        initialization_time{std::chrono::steady_clock::now()}
{
    this->device_id = device_id.c_str();
    // Initialize RecordContext
    this->rc.ring_buffer = nullptr;
};

UsbMicro::~UsbMicro() {
    if (mic_in_stream) {
        soundio_instream_destroy(mic_in_stream);
    }
    if (microphone) {
        soundio_device_unref(microphone);
    }
    if (soundio) {
        soundio_destroy(soundio);
    }
}

static void read_callback(struct SoundIoInStream* instream, int frame_count_min, int frame_count_max) {
    static int callback_count = 0;
    callback_count++;
    
    struct RecordContext *rc = (RecordContext*) instream->userdata;
    struct SoundIoChannelArea *areas;
    int err;

    char *write_ptr = soundio_ring_buffer_write_ptr(rc->ring_buffer);
    int free_bytes = soundio_ring_buffer_free_count(rc->ring_buffer);
    int free_count = free_bytes / instream->bytes_per_frame;

    if (free_count < frame_count_min) {
        fprintf(stderr, "ring buffer overflow\n");
        return;
    }

    int write_frames = (free_count < frame_count_max) ? free_count : frame_count_max;
    int frames_left = write_frames;

    for (;;) {
        int frame_count = frames_left;

        if ((err = soundio_instream_begin_read(instream, &areas, &frame_count))) {
            fprintf(stderr, "begin read error: %s", soundio_strerror(err));
            break;
        }

        if (!frame_count) {
            break;
        }

        if (!areas) {
            // Due to an overflow there is a hole. Fill the ring buffer with
            // silence for the size of the hole.
            memset(write_ptr, 0, frame_count * instream->bytes_per_frame);
        } else {
            for (int frame = 0; frame < frame_count; frame += 1) {
                for (int ch = 0; ch < instream->layout.channel_count; ch += 1) {
                    memcpy(write_ptr, areas[ch].ptr, instream->bytes_per_sample);
                    areas[ch].ptr += areas[ch].step;
                    write_ptr += instream->bytes_per_sample;
                }
            }
        }

        if ((err = soundio_instream_end_read(instream))) {
            fprintf(stderr, "end read error: %s", soundio_strerror(err));
            return;
        }

        frames_left -= frame_count;
        if (frames_left <= 0) {
            break;
        }
    }

    int advance_bytes = write_frames * instream->bytes_per_frame;
    soundio_ring_buffer_advance_write_ptr(rc->ring_buffer, advance_bytes);
}

SoundIoFormat find_supported_format(SoundIoDevice *microphone) {
    // Try to find a supported format
    SoundIoFormat supported_format;
    SoundIoFormat formats[] = {SoundIoFormatS16NE, SoundIoFormatS24NE, SoundIoFormatS32NE, SoundIoFormatFloat32NE};
    bool format_found = false;
    for (int i = 0; i < 4; i++) {
        if (soundio_device_supports_format(microphone, formats[i])) {
            supported_format = formats[i];
            format_found = true;
            std::cout << "Using format: " << soundio_format_string(supported_format) << std::endl;
            break;
        }
    }
    if (!format_found) {
        return SoundIoFormatInvalid;
    }
    return supported_format;
}

int find_supported_sample_rate(SoundIoDevice *microphone) {
    // Try common sample rates
    int rates[] = {8000, 16000, 22050, 48000};
    int supported_sample_rate;
    bool rate_found = false;
    for (int i = 0; i < 4; i++) {
        if (soundio_device_supports_sample_rate(microphone, rates[i])) {
            supported_sample_rate = rates[i];
            rate_found = true;
            std::cout << "Using sample rate: " << supported_sample_rate << std::endl;
            break;
        }
    }
    if (!rate_found) {
        return NO_SUPPORTED_SAMPLE_RATE;
    }

    return supported_sample_rate;
}

void UsbMicro::initialize() {
    bool in_raw = false;
    int sample_rate = 44100;
    SoundIoFormat prioritized_format = SoundIoFormatS16NE;

    this->soundio = soundio_create();
    if (!soundio) {
        std::cout << "out of memory" << std::endl;
        return;
    }

    int err = soundio_connect(soundio);
    if (err) {
        std::cout << "error connecting " << soundio_strerror(err) << std::endl;
        return;
    }
    soundio_flush_events(soundio);

    int input_device_count = soundio_input_device_count(soundio);
    if (input_device_count < 0) {
        std::cout << "no input device found" << std::endl;
        return;
    }

    // Try to connect to input device_id
    int in_device_index = 0;
    bool found = false;
    for (int i = 0; i < input_device_count; i += 1) {
        struct SoundIoDevice *device = soundio_get_input_device(soundio, i);
        if (device->is_raw == in_raw && strcmp(device->id, this->device_id) == 0) {
            std::cout << "Found device: " << device->name << std::endl;
            in_device_index = i;
            found = true;
            soundio_device_unref(device);
            break;
        } else {
            std::cout << "Device: " << device->name << " is not found" << std::endl;
        }
        soundio_device_unref(device);
    }
    if (!found) {
        std::cout << "invalid input device id " << this->device_id << std::endl;
        return;
    }

    microphone = soundio_get_input_device(soundio, in_device_index);
    if (!microphone) {
        std::cout << "could not get input device: out of memory" << std::endl;
        return;
    }
    std::cout << "Got in device: " << microphone->name << std::endl;

    if (microphone->probe_error) {
        fprintf(stderr, "Unable to probe device: %s\n", soundio_strerror(microphone->probe_error));
        return;
    }

    soundio_device_sort_channel_layouts(microphone);

    // Check format compatibility and fallback if needed
    if (!soundio_device_supports_format(microphone, prioritized_format)) {
        std::cout << "microphone does not support format " << soundio_format_string(prioritized_format) << std::endl;
        prioritized_format = find_supported_format(this->microphone);
        if (prioritized_format == SoundIoFormatInvalid) {
            std::cout << "No supported format found" << std::endl;
            return;
        }
    }

    if (!soundio_device_supports_sample_rate(microphone, sample_rate)) {
        std::cout << "microphone does not support sample rate " << sample_rate << std::endl;
        sample_rate = find_supported_sample_rate(this->microphone);
        if (sample_rate == NO_SUPPORTED_SAMPLE_RATE) {
            std::cout << "No supported sample rate found" << std::endl;
            return;
        }
    }

    mic_in_stream = soundio_instream_create(microphone);
    if (!mic_in_stream) {
        std::cout << "out of memory instream" << std::endl;
        return;
    }
    mic_in_stream->format = prioritized_format;
    mic_in_stream->sample_rate = sample_rate;
    mic_in_stream->read_callback = &read_callback;
    mic_in_stream->userdata = &this->rc;

    if ((err = soundio_instream_open(mic_in_stream))) {
        fprintf(stderr, "unable to open input stream: %s", soundio_strerror(err));
        return;
    }

    fprintf(stderr, "%s %dHz %s interleaved\n",
            mic_in_stream->layout.name, sample_rate, soundio_format_string(prioritized_format));

    const int ring_buffer_duration_seconds = 6;
    int capacity = ring_buffer_duration_seconds * mic_in_stream->sample_rate * mic_in_stream->bytes_per_frame;
    std::cout << "capacity: " << capacity << std::endl;
    rc.ring_buffer = soundio_ring_buffer_create(soundio, capacity);
    if (!rc.ring_buffer) {
        fprintf(stderr, "out of memory\n");
        return;
    }

    if ((err = soundio_instream_start(mic_in_stream))) {
        std::cout << "unable to start input device: " << soundio_strerror(err) << std::endl;
        return;
    }
}

bool UsbMicro::read(AudioBuffer* buffer) {
    soundio_flush_events(soundio);
    
    // Check if the stream is still running
    if (!mic_in_stream || !rc.ring_buffer) {
        std::cout << "Stream not initialized or has been destroyed" << std::endl;
        return false;
    }
    
    int fill_bytes = soundio_ring_buffer_fill_count(rc.ring_buffer);
    char *read_buf = soundio_ring_buffer_read_ptr(rc.ring_buffer);
    
    static int consecutive_empty_reads = 0;
    
    if (fill_bytes == 0) {
        /*consecutive_empty_reads++;*/
        /**/
        /*// If we've had too many consecutive empty reads, the stream might be dead*/
        /*if (consecutive_empty_reads > 10) {*/
        /*    std::cout << "Stream appears to be dead. Attempting to restart..." << std::endl;*/
        /*    // Try to restart the stream*/
        /*    soundio_instream_start(mic_in_stream);*/
        /*    consecutive_empty_reads = 0;*/
        /*}*/
        
        return false;
    }
    
    consecutive_empty_reads = 0; // Reset counter when we get data

    // Calculate how many samples we can read
    int bytes_per_sample = mic_in_stream->bytes_per_sample;
    int samples_available = fill_bytes / bytes_per_sample;
    int samples_to_read = std::min(samples_available, (int)I2S_DMA_BUF_LEN);
    
    buffer->num_samples = samples_to_read;
    buffer->bytes_read = samples_to_read * bytes_per_sample;
    buffer->samplingRate = mic_in_stream->sample_rate;

    // Convert samples based on format
    for (int i = 0; i < samples_to_read; i++) {
        double sample_value = 0.0;
        
        switch (mic_in_stream->format) {
            case SoundIoFormatS16NE: {
                // 16-bit signed integer, native endian
                int16_t sample;
                memcpy(&sample, read_buf + (i * bytes_per_sample), sizeof(int16_t));
                sample_value = (double)sample / 32768.0; // Normalize to [-1.0, 1.0]
                break;
            }
            case SoundIoFormatS24NE: {
                // 24-bit signed integer, native endian
                int32_t sample = 0;
                memcpy(&sample, read_buf + (i * bytes_per_sample), 3);
                // Handle endianness for 24-bit
                if (sample & 0x800000) {
                    sample |= 0xFF000000; // Sign extend
                }
                sample_value = (double)sample / 8388608.0; // Normalize to [-1.0, 1.0]
                break;
            }
            case SoundIoFormatS32NE: {
                // 32-bit signed integer, native endian
                int32_t sample;
                memcpy(&sample, read_buf + (i * bytes_per_sample), sizeof(int32_t));
                sample_value = (double)sample / 2147483648.0; // Normalize to [-1.0, 1.0]
                break;
            }
            case SoundIoFormatFloat32NE: {
                // 32-bit float, native endian
                float sample;
                memcpy(&sample, read_buf + (i * bytes_per_sample), sizeof(float));
                sample_value = (double)sample;
                break;
            }
            default:
                // Unsupported format, use silence
                sample_value = 0.0;
                break;
        }
        
        buffer->buffer[i] = sample_value;
    }

    soundio_ring_buffer_advance_read_ptr(rc.ring_buffer, samples_to_read * bytes_per_sample);
    return true;
}

void UsbMicro::print_available_input_devices() {
    struct SoundIo *soundio = soundio_create();
    if (!soundio) {
        std::cout << "out of memory" << std::endl;
        return;
    }
    int err = soundio_connect(soundio);
    if (err) {
        std::cout << "error connecting: " << soundio_strerror(err) << std::endl;
        soundio_destroy(soundio);
        return;
    }
    soundio_flush_events(soundio);
    int num_devices = soundio_input_device_count(soundio);
    std::cout << "Available input devices (" << num_devices << "):" << std::endl;
    for (int i = 0; i < num_devices; ++i) {
        SoundIoDevice* dev = soundio_get_input_device(soundio, i);
        std::cout << "  [" << i << "] Name: '" << dev->name << "', ID: '" << dev->id << "'" << std::endl;
        soundio_device_unref(dev);
    }
    soundio_destroy(soundio);
}
