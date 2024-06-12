import scipy.signal as signal
from wav import Wave
import numpy as np

lowcut = 100
order = 4

def filter_lowpass_scipy(wave: Wave, lowcut=100, order=4):
    """
    Applies a butterworth lowpass filter to a wave signal with the second order sections algorithm.
    lowcut: Frequency to cut off
    order: Order of the filter
    """
    sos = signal.butter(order, lowcut, 'low', fs=wave.framerate, output='sos')
    ys = wave.ys
    filtered = signal.sosfilt(sos, ys)
    return Wave(filtered, wave.ts, wave.framerate)

def filter_bandpass_scipy(wave: Wave, lowcut=20, highcut=100, order=4):
    """
    Applies a butterworth bandpass filter to a wave signal with the second order sections algorithm.
    lowcut: lower frequency to cut off
    highcut: higher frequency to cut off
    order: Order of the filter
    """
    sos = signal.butter(order, [lowcut, highcut], 'band', fs=wave.framerate, output='sos')
    ys = wave.ys
    filtered = signal.sosfilt(sos, ys)
    return Wave(filtered, wave.ts, wave.framerate)

def filter_lowpass_ab_scipy(wave: Wave):
    """
    Applies a butterworth lowpass filter to a wave signal using numerator/denominator.
    lowcut: lower frequency to cut off
    highcut: higher frequency to cut off
    order: Order of the filter
    """
    b, a = signal.butter(order, lowcut, 'low', fs=wave.framerate)
    filtered = signal.filtfilt(b, a, wave.ys)
    return Wave(filtered, wave.ts, wave.framerate)


prev_filtered = [0] * 3
prev_samples = [0] * 3
def iir_filter_sample_order_2(b, a, sample):
    """
    Implements an Infinite Impulse Response (IIR) filter algorithm
    that applies the second order numerator/denominator iir filter to a single sample
    of a continuous signal

    This is a second order section that is applied

    b: numerator
    a: denominator
    sample: a single sample
    """
    global prev_filtered
    global prev_samples
    prev_samples[0] = prev_samples[1]
    prev_samples[1] = prev_samples[2]
    prev_samples[2] = sample
    prev_filtered[0] = prev_filtered[1]
    prev_filtered[1] = prev_filtered[2]

    filtered_sample = 0
    filtered_sample += b[0] * prev_samples[2]
    filtered_sample += b[1] * prev_samples[1]
    filtered_sample += b[2] * prev_samples[0]

    filtered_sample -= a[1] * prev_filtered[1]
    filtered_sample -= a[2] * prev_filtered[0]

    prev_filtered[2] = filtered_sample

    return filtered_sample


def iir_filter(b, a, sig):
    """
    Convenience function to apply an iir filter to a signal with given numerator/denominator.
    The filter is of order 2 because second order section is applied
    """
    filtered_signal = [0] * len(sig)
    for n in range(len(sig)):
        filtered_signal[n] = iir_filter_sample_order_2(b, a, sig[n])
    return filtered_signal

def filter_lowpass_ab_own_order_2(wave: Wave):
    """
    Showcase how a second order iir filter is applied to a wave
    """
    b, a = signal.butter(2, lowcut, 'low', fs=wave.framerate)
    filtered = iir_filter(b, a, wave.ys)
    filtered_2 = iir_filter(b, a, filtered[::-1])
    return Wave(filtered_2[::-1], wave.ts, wave.framerate)

def filter_bandpass(wave: Wave, lowcut=20, highcut=100, order=4):
    """
    Applies a bandpass filter to wave using second order sections
    """
    sos = signal.butter(order, [lowcut, highcut], 'band', fs=wave.framerate, output='sos')
    filtered_signal = wave.ys
    for section in range(len(sos)):
        b = [sos[section][0], sos[section][1], sos[section][2]]
        a = [sos[section][3], sos[section][4], sos[section][5]]
        filtered_signal = iir_filter(b, a, filtered_signal)
    return Wave(filtered_signal, wave.ts, wave.framerate)

def filter_bandpass_sample(sample, sos):
    """
    Applies a bandpass filter to a sample using second order sections

    This implementation is in the end used in the ESP32 firmware
    """
    filtered_sample = sample
    for section in range(len(sos) - 1):
        b = [sos[section][0], sos[section][1], sos[section][2]]
        a = [sos[section][3], sos[section][4], sos[section][5]]
        # filtered_sample = iir_filter(b, a, filtered_sample)
        filtered_sample = iir_filter_sample_order_2(b, a, filtered_sample)
    return filtered_sample

