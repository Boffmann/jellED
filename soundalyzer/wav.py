import numpy as np
from wave import open as open_wave
import subprocess
import warnings
import datetime

class WavFileWriter:
    """Writes wav files."""

    def __init__(self, filename="sound.wav", framerate=11025):
        """Opens the file and sets parameters.

        filename: string
        framerate: samples per second
        """
        self.filename = filename
        self.framerate = framerate
        self.nchannels = 1
        self.sampwidth = 2
        self.bits = self.sampwidth * 8
        self.bound = 2 ** (self.bits - 1) - 1

        self.fmt = "h"
        self.dtype = np.int16

        self.fp = open_wave(self.filename, "w")
        self.fp.setnchannels(self.nchannels)
        self.fp.setsampwidth(self.sampwidth)
        self.fp.setframerate(self.framerate)

    def write(self, wave):
        """Writes a wave.

        wave: Wave
        """
        zs = wave.quantize(self.bound, self.dtype)
        self.fp.writeframes(zs.tostring())

    def close(self, duration=0):
        """Closes the file.

        duration: how many seconds of silence to append
        """
        if duration:
            self.write(rest(duration))

        self.fp.close()

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
        self.ys = amp * self.ys / max(high, low)

    def quantize(self, bound, dtype):
        """Maps the waveform to quanta.

        bound: maximum amplitude
        dtype: numpy data type or string

        returns: quantized signal
        """
        if max(self.ys) > 1 or min(self.ys) < -1:
            warnings.warn("Warning: normalizing before quantizing.")
            self.normalize(self.ys)

        zs = (self.ys * bound).astype(dtype)
        return zs

    def write(self, filename="sound.wav"):
        """Write a wave file.

        filename: string
        """
        print("Writing", filename)
        wfile = WavFileWriter(filename, self.framerate)
        wfile.write(self)
        wfile.close()

    def play(self, filename="sound.wav"):
        now = datetime.datetime.now()
        self.write(filename)
        print("passed:")
        print(datetime.datetime.now() - now)
        self.play_wave(filename)

    def play_wave(self, filename="sound.wav", player="afplay"):
        """Plays a wave file.

        filename: string
        player: string name of executable that plays wav files
        """
        cmd = "%s %s" % (player, filename)
        popen = subprocess.Popen(cmd, shell=True)
        popen.communicate()


def wave_from_wav(filepath):
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

def wave_from_recorded_file(filepath, framerate, recorded_seconds, span_to_seconds):
    ys = []
    with open(filepath, mode="r") as input_file:
        for line in input_file.readlines():
            ys.append(float(line.strip('\n')))

    repeat_count = int(span_to_seconds / recorded_seconds)
    total_samples = framerate * recorded_seconds
    for i in range(repeat_count):
        for j in range(total_samples):
            ys.append(ys[j])
    return Wave(ys, framerate=framerate)

