#ifndef _USB_MICRO_RASPI_JELLED_H__
#define _USB_MICRO_RASPI_JELLED_H__

#include "sound/soundinput.h"

#include <string>

#include <soundio/soundio.h>
#include <stdio.h>
#include <chrono>
#include <thread>
#include <atomic>

struct RecordContext {
    struct SoundIoRingBuffer* ring_buffer;
};

class UsbMicro : public SoundInput {
private:
    struct SoundIo *soundio;
    const char* device_id;
    struct SoundIoDevice *microphone;
    struct SoundIoInStream *mic_in_stream;
    struct RecordContext rc;
    std::chrono::steady_clock::time_point initialization_time;
    std::thread event_thread;
    std::atomic<bool> event_thread_running;

public:
    UsbMicro(std::string& device_id);
    ~UsbMicro();
    void initialize();
    bool read(AudioBuffer* buffer);
    static void print_available_input_devices();
};

#endif
