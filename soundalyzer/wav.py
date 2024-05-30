import numpy as np
from wave import open as open_wave

class Wave():
    """Represents a discrete-time waveform"""
    def __init__(self, ys, ts=None, framerate=None):
        """Initializes the wave.

        ys: wave array
        ts: array of times
        framerate: samples per second
        """
        self.ys = np.asanyarray(ys)
        self.framerate = framerate if framerate is not None else 11025

        if ts is None:
            self.ts = np.arange(len(ys)) / self.framerate
        else:
            self.ts = np.asanyarray(ts)

    def normalize(self, amp=1.0):
        """Normalizes the signal to the given amplitude.

        amp: float amplitude
        """
        high, low = abs(max(self.ys)), abs(min(self.ys))
        return amp * self.ys / max(high, low)
        # self.ys = normalize(self.ys, amp=amp)


def read_wave(filepath):
        fp = open_wave(filepath, 'r')

        nchannels = fp.getnchannels()
        nframes = fp.getnframes()
        sampwidth = fp.getsampwidth()
        framerate = fp.getframerate()

        z_str = fp.readframes(nframes)

        fp.close()

        dtype_map = {1: np.int8, 2: np.int16, 3: "special", 4: np.int32}
        if sampwidth not in dtype_map:
            raise ValueError("sampwidth %d unknown" % sampwidth)

        if sampwidth == 3:
            xs = np.fromstring(z_str, dtype=np.int8).astype(np.int32)
            ys = (xs[2::3] * 256 + xs[1::3]) * 256 + xs[0::3]
        else:
            ys = np.fromstring(z_str, dtype=dtype_map[sampwidth])

        # if it's in stereo, just pull out the first channel
        if nchannels == 2:
            ys = ys[::2]

        # ts = np.arange(len(ys)) / framerate
        wave = Wave(ys, framerate=framerate)
        wave.normalize()
        return wave


