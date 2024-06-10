import scipy.signal as signal
from wav import Wave
import numpy as np

sample_rate = 48000
lowcut = 100
# highcut = X.X
order = 4


def filter_lowpass_scipy(wave: Wave):
    sos = signal.butter(order, lowcut, 'low', fs=sample_rate, output='sos')
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
        # sum_b = 0
        # sum_a = 0
        # for i in range(order):
        #     sig_value = 0.0 if n - i < 0 else sig[n - i]
        #     sum_b += b[i] * sig_value
        # for j in range(1, order):
        #     sig_value = 0.0 if n - j < 0 else filtered_signal[n - j]
        #     sum_a += a[j] * sig_value

        # filtered_signal[n] = sum_b - sum_a
        # filtered_signal[n] *= (1.0 / a[0])
    # return filtered_signal


def filter_lowpass_ab_own(wave: Wave):
    b, a = signal.butter(order, lowcut, 'low', fs=sample_rate)
    filtered = iir_filter(b, a, wave.ys)
    filtered_2 = iir_filter(b, a, filtered[::-1])
    return Wave(filtered_2[::-1], wave.ts, wave.framerate)

def filter_lowpass_own(wave: Wave):
    sos = [[ 1.80397952e-09, 3.60795904e-09, 1.80397952e-09, 1.00000000e+00,
            -1.97593328e+00, 9.76102578e-01],
            [ 1.00000000e+00, 2.00000000e+00, 1.00000000e+00, 1.00000000e+00,
            -1.98986110e+00, 9.90031591e-01]]
    # sos = signal.butter(order, lowcut, 'low', fs=sample_rate, output='sos')
    filtered_signal = wave.ys
    for section in range(2):
        b = [sos[section][0], sos[section][1], sos[section][2]]
        a = [sos[section][3], sos[section][4], sos[section][5]]
        filtered_signal = iir_filter(b, a, filtered_signal)
    return Wave(filtered_signal, wave.ts, wave.framerate)

    # ys = np.zeros_like(wave.ts)
    # # z = [[0, 0], [0, 0]]
    # # z = np.zeros((sos.shape[0], 2))
    # result = []
    # intermediate = []
    # # for section in range(2):
    # section = 0
    # b = [sos[section][0], sos[section][1], sos[section][2]]
    # a = [sos[section][3], sos[section][4], sos[section][5]]
    # for n in range(len(wave.ts)):
    #     b_sum = 0
    #     a_sum = 0
    #     for i in range(2):
    #         if n - i < 0:
    #             continue
    #         b_sum += b[i] * sig[n - i]
    #     for j in range(1, 2, 1):
    #         if n - j < 0:
    #             continue
    #         b_sum += a[j] * sig[n - j]

    #     intermediate.append((1.0 / a[0]) * (b_sum - a_sum))

    # section = 1
    # b = [sos[section][0], sos[section][1], sos[section][2]]
    # a = [sos[section][3], sos[section][4], sos[section][5]]
    # for n in range(len(wave.ts)):
    #     b_sum = 0
    #     a_sum = 0
    #     for i in range(2):
    #         if n - i < 0:
    #             continue
    #         b_sum += b[i] * intermediate[n - i]
    #     for j in range(1, 2, 1):
    #         if n - j < 0:
    #             continue
    #         b_sum += a[j] * intermediate[n - j]

    #     result.append((1.0 / a[0]) * (b_sum - a_sum))

    # return Wave(result, wave.ts, wave.framerate)

