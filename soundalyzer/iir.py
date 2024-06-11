import scipy.signal as signal
from wav import Wave
import numpy as np

sample_rate = 48000
lowcut = 100
# highcut = X.X
order = 4

def filter_lowpass_fir(wave: Wave):
    ys = np.absolute(wave.ys)
    order=4
    sos = signal.butter(order, 100, 'low', fs=sample_rate, output='sos')
    filtered = signal.sosfilt(sos, ys)
    return Wave(filtered, wave.ts, wave.framerate)
    # nyq_rate = wave.framerate / 2.0
    # width = 5.0/nyq_rate
    # ripple_db = 60.0
    # N, beta = signal.kaiserord(ripple_db, width)
    # cutoff_hz = 30.0
    # taps = signal.firwin(N, cutoff_hz/nyq_rate, window=('kaiser', beta))
    # filtered = signal.lfilter(taps, 1.0, wave.ys)
    # return Wave(filtered, wave.ts, wave.framerate)

def filter_lowpass_scipy(wave: Wave):
    sos = signal.butter(order, lowcut, 'low', fs=sample_rate, output='sos')
    ys = wave.ys
    filtered = signal.sosfilt(sos, ys)
    return Wave(filtered, wave.ts, wave.framerate)

def filter_bandpass_scipy(wave: Wave):
    sos = signal.butter(order, [20, 100], 'band', fs=sample_rate, output='sos')
    ys = wave.ys
    filtered = signal.sosfilt(sos, ys)
    return Wave(filtered, wave.ts, wave.framerate)

def filter_lowpass_ab_scipy(wave: Wave):
    b, a = signal.butter(order, lowcut, 'low', fs=sample_rate)
    filtered = signal.filtfilt(b, a, wave.ys)
    return Wave(filtered, wave.ts, wave.framerate)


prev_filtered = [0] * 3
prev_samples = [0] * 3
def iir_filter_sample_order_2(b, a, sample):
    prev_samples[0] = prev_samples[1]
    prev_samples[1] = prev_samples[2]
    prev_samples[2] = sample
    prev_filtered[0] = prev_filtered[1]
    prev_filtered[1] = prev_filtered[2]

    filtered_signal = 0
    filtered_signal += b[0] * prev_samples[2]
    filtered_signal += b[1] * prev_samples[1]
    filtered_signal += b[2] * prev_samples[0]

    filtered_signal -= a[1] * prev_filtered[1]
    filtered_signal -= a[2] * prev_filtered[0]

    prev_filtered[2] = filtered_signal

    return filtered_signal


def iir_filter(b, a, sig):
    filtered_signal = [0] * len(sig)
    order = len(b)
    for n in range(len(sig)):
        filtered_signal[n] = iir_filter_sample_order_2(b, a, sig[n])
    return filtered_signal


def filter_lowpass_ab_own(wave: Wave):
    b, a = signal.butter(order, lowcut, 'low', fs=sample_rate)
    filtered = iir_filter(b, a, wave.ys)
    filtered_2 = iir_filter(b, a, filtered[::-1])
    return Wave(filtered_2[::-1], wave.ts, wave.framerate)

def filter_bandpass_own(wave: Wave):
    sos = [[ 7.41426642e-10,  1.48285328e-09,  7.41426642e-10,  1.00000000e+00,
        -1.98596111e+00,  9.86051254e-01],
        [ 1.00000000e+00,  2.00000000e+00,  1.00000000e+00,  1.00000000e+00,
        -1.99324771e+00,  9.93408841e-01],
        [ 1.00000000e+00, -2.00000000e+00,  1.00000000e+00,  1.00000000e+00,
        -1.99469814e+00,  9.94711042e-01],
        [ 1.00000000e+00, -2.00000000e+00,  1.00000000e+00,  1.00000000e+00,
        -1.99859190e+00,  9.98599159e-01]]
    # sos = signal.butter(order, [20, 100], 'band', fs=sample_rate, output='sos')
    filtered_signal = wave.ys
    for section in range(len(sos) - 1):
        b = [sos[section][0], sos[section][1], sos[section][2]]
        a = [sos[section][3], sos[section][4], sos[section][5]]
        filtered_signal = iir_filter(b, a, filtered_signal)
    return Wave(filtered_signal, wave.ts, wave.framerate)

