import scipy.signal as signal
import numpy as np
from wav import Wave
import collections

class EnvelopeDetector:
    def __init__(self, frame_size, hop_length):
        self.frame_size = frame_size
        self.hop_length = hop_length
        # Use a "circular buffer" that automatically moves new elements in from the back see
        # https://stackoverflow.com/questions/4151320/efficient-circular-buffer
        self.previous_samples = collections.deque(maxlen=frame_size)

        self.buffered_since_last_result = 0
        self.window_max = 0

    def envelope_sample(self, sample):
        """
        Calculates the envelope based on a backwards looking approach.
        Every hop_length, the last frame_size many buffered samples are looked at and the max is taken
        This max is used as the envelope value until a new max is caluclated at the next hop_length many samples
        """
        self.previous_samples.append(sample)
        self.buffered_since_last_result += 1
        if self.buffered_since_last_result == self.hop_length:
            self.buffered_since_last_result = 0
            self.window_max = 0
            for j in range(self.frame_size):
                backward_sample = 0 if j >= len(self.previous_samples) else self.previous_samples[j]
                self.window_max = max(self.window_max, backward_sample)

        return self.window_max

    def filter_envelope_backward(self, wave: Wave):
        """
        Calculates an envelope by taking the max of frame_size many previous samples every
        hop_length. This effectively downsamples the envelope
        """
        result = []
        for i in range(0, wave.ys.size, self.hop_length):
            window_max = 0
            for j in range(self.frame_size):
                backward_sample = 0 if i-j < 0 else wave.ys[i-j]
                window_max = max(window_max, backward_sample)
            result.append(window_max)

        ts = np.arange(0, wave.ts[-1], wave.ts[-1] / len(result))
        return ts, result

    def envelope_with_lowpass(self, wave: Wave):
        """
        Calculated the envelope of a wave by rectifying the signal and applying a lowpass filter.

        This has the drawback that it shifts the enveloped a couple of samples to the right
        """
        ys = np.absolute(wave.ys)
        order=4
        sos = signal.butter(order, 10, 'low', fs=sample_rate, output='sos')
        filtered = signal.sosfilt(sos, ys)
        return Wave(filtered, wave.ts, wave.framerate)

    def filter_envelope_numpy(self, wave: Wave):
        """
        The numpy implementation of the forward looking approach that is manually implemented if __name__ == "__main__":
        "filter_envelope_backward"
        """
        frame_size = 1024
        hop_length = 512
        result = np.array([max(wave.ys[i:i+frame_size]) for i in range(0, wave.ys.size, hop_length)])
        print("Result length: " + str(len(result)))
        ts = np.arange(0, wave.ts[-1], wave.ts[-1] / len(result))
        return ts, result
