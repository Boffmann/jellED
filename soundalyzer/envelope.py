import scipy.signal as signal
from wav import Wave

def filter_lowpass_fir(wave: Wave):
    ys = np.absolute(wave.ys)
    order=4
    sos = signal.butter(order, 10, 'low', fs=sample_rate, output='sos')
    filtered = signal.sosfilt(sos, ys)
    return Wave(filtered, wave.ts, wave.framerate)
