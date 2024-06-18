import scipy.signal as signal
import collections

class BandpassFilter:
    def __init__(self, order, lowcut, highcut, framerate):
        self.sos = signal.butter(order, [lowcut, highcut], 'band', fs=framerate, output='sos')
        self.prev_samples_per_section = {}
        self.prev_filtered_per_section = {}
        self.fps = framerate
        for section in range(len(self.sos)):
            self.prev_samples_per_section[section] = collections.deque(maxlen=3)
            self.prev_filtered_per_section[section] = collections.deque(maxlen=3)
            for i in range(3):
                self.prev_samples_per_section[section].append(0)
                self.prev_filtered_per_section[section].append(0)

        self.should_downsample = False
        self.downsampling_counter = 0
        self.downsampling_threshold = None

    def _filter_sample_sos(self, sample, section):
        """
        Applies a second order section iir convelution filter to a sample

        It works by tracking both previous samples as well as previously filtered samples
        per section. This is required because the samples come in one after another.
        Each section must not be conveluted with samples/filtered samples from other sections

        This method should not be called directly. Instead, use "iir_filter_sos"
        """
        b = [self.sos[section][0], self.sos[section][1], self.sos[section][2]]
        a = [self.sos[section][3], self.sos[section][4], self.sos[section][5]]

        # self.prev_samples_per_section[section][0] = self.prev_samples_per_section[section][1]
        # self.prev_samples_per_section[section][1] = self.prev_samples_per_section[section][2]
        # self.prev_samples_per_section[section][2] = sample
        self.prev_samples_per_section[section].append(sample)

        # self.prev_filtered_per_section[section][0] = self.prev_filtered_per_section[section][1]
        # self.prev_filtered_per_section[section][1] = self.prev_filtered_per_section[section][2]

        filtered_sample = 0

        filtered_sample += b[0] * self.prev_samples_per_section[section][2]
        filtered_sample += b[1] * self.prev_samples_per_section[section][1]
        filtered_sample += b[2] * self.prev_samples_per_section[section][0]

        # filtered_sample -= a[1] * self.prev_filtered_per_section[section][1]
        # filtered_sample -= a[2] * self.prev_filtered_per_section[section][0]
        filtered_sample -= a[1] * self.prev_filtered_per_section[section][2]
        filtered_sample -= a[2] * self.prev_filtered_per_section[section][1]

        self.prev_filtered_per_section[section].append(filtered_sample)

        return filtered_sample

    def iir_filter_sos(self, sample):
        filtered_sample = sample
        for section in range(len(self.sos)):
            filtered_sample = self._filter_sample_sos(filtered_sample, section)

        # In case of downsampling, all samples must be filtered to get accurate results but only after a certain
        # number of samples is filtered, a new value is returned to get a downsampling effect
        if self.should_downsample:
            self.downsampling_counter += 1
            if self.downsampling_counter >= self.downsampling_threshold:
                self.downsampling_counter = 0
                return filtered_sample
            else:
                return None
        return filtered_sample

    def activate_downsampling(self, desired_fps):
        if self.fps < desired_fps:
            print("Cannot activate downsampling if desired_fps is larger")
            return
        self.should_downsample = True
        self.downsampling_threshold = self.fps / desired_fps

