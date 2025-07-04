import scipy.signal as signal
import numpy as np
from wav import Wave
import collections


class EnvelopeDetector:
    def __init__(self, frame_size, hop_length, downsample_factor=1):
        self.frame_size = frame_size
        self.hop_length = hop_length
        self.downsample_factor = downsample_factor
        # Use a "circular buffer" that automatically moves new elements in from the back see
        # https://stackoverflow.com/questions/4151320/efficient-circular-buffer
        self.previous_samples = collections.deque(maxlen=frame_size)

        self.buffered_since_last_result = 0
        self.window_max = 0

        # For real-time envelope detection
        self.current_envelope = 0.0

        # For downsampled processing
        self.sample_counter = 0
        self.downsampled_buffer = []

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
                backward_sample = 0 if j >= len(
                    self.previous_samples) else self.previous_samples[j]
                self.window_max = max(self.window_max, backward_sample)

        return self.window_max

    def envelope_sample_downsampled(self, sample):
        """
        Downsampled version of envelope_sample that processes every Nth sample.
        This reduces computational load while maintaining envelope detection accuracy.
        """
        self.sample_counter += 1

        # Only process every downsample_factor samples
        if self.sample_counter % self.downsample_factor != 0:
            return self.window_max

        # Process the sample normally
        return self.envelope_sample(sample)

    def filter_envelope_backward(self, wave: Wave, downsample_factor=None):
        """
        Calculates an envelope by taking the max of frame_size many previous samples every
        hop_length. This effectively downsamples the envelope.

        Args:
            wave: Input wave signal
            downsample_factor: Factor by which to downsample the envelope (1=no downsampling, 2=half rate, etc.)
        """
        if downsample_factor is None:
            downsample_factor = self.downsample_factor

        if downsample_factor <= 1:
            # Original implementation
            result = []
            for i in range(0, wave.ys.size, self.hop_length):
                window_max = 0
                for j in range(self.frame_size):
                    backward_sample = 0 if i-j < 0 else wave.ys[i-j]
                    window_max = max(window_max, backward_sample)
                result.append(window_max)
        else:
            # Downsampled implementation for efficiency
            result = []
            effective_hop = self.hop_length * downsample_factor

            for i in range(0, wave.ys.size, effective_hop):
                window_max = 0
                # Look at a larger window to compensate for downsampling
                window_size = self.frame_size * downsample_factor
                for j in range(window_size):
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
        order = 4
        sos = signal.butter(order, 10, 'low', fs=wave.framerate, output='sos')
        filtered = signal.sosfilt(sos, ys)
        return Wave(filtered, wave.ts, wave.framerate)

    def filter_envelope_numpy(self, wave: Wave, downsample_factor=None):
        """
        The numpy implementation of the forward looking approach that is manually implemented if __name__ == "__main__":
        "filter_envelope_backward"

        Args:
            wave: Input wave signal
            downsample_factor: Factor by which to downsample the envelope (1=no downsampling, 2=half rate, etc.)
        """
        if downsample_factor is None:
            downsample_factor = self.downsample_factor

        frame_size = 1024
        hop_length = 512 * downsample_factor  # Adjust hop length for downsampling

        if downsample_factor <= 1:
            result = np.array([max(wave.ys[i:i+frame_size])
                              for i in range(0, wave.ys.size, hop_length)])
        else:
            # More efficient downsampled version
            result = []
            for i in range(0, wave.ys.size, hop_length):
                end_idx = min(i + frame_size, wave.ys.size)
                window_max = np.max(wave.ys[i:end_idx])
                result.append(window_max)
            result = np.array(result)

        print("Result length: " + str(len(result)))
        ts = np.arange(0, wave.ts[-1], wave.ts[-1] / len(result))
        return ts, result

    def envelope_rectification_lowpass(self, wave: Wave, cutoff_freq=10.0, order=4, downsample_factor=None):
        """
        Standard envelope detection using rectification + low-pass filtering.
        This is the most common and reliable method for envelope detection.
        Now includes optional downsampling for improved efficiency.

        Args:
            wave: Input wave signal
            cutoff_freq: Low-pass filter cutoff frequency (Hz)
            order: Filter order
            downsample_factor: Factor by which to downsample the envelope (1=no downsampling, 2=half rate, etc.)

        Returns:
            Wave object containing the envelope
        """
        if downsample_factor is None:
            downsample_factor = self.downsample_factor

        # Step 1: Rectify the signal (take absolute value)
        rectified = np.abs(wave.ys)

        # Step 2: Apply low-pass filter to smooth the rectified signal
        sos = signal.butter(order, cutoff_freq, 'low',
                            fs=wave.framerate, output='sos')
        envelope = signal.sosfilt(sos, rectified)

        # Step 3: Downsample if requested
        if downsample_factor > 1:
            # Apply anti-aliasing filter before downsampling
            anti_alias_cutoff = wave.framerate / (2 * downsample_factor)
            anti_alias_sos = signal.butter(
                order, anti_alias_cutoff, 'low', fs=wave.framerate, output='sos')
            envelope = signal.sosfilt(anti_alias_sos, envelope)

            # Downsample
            envelope = envelope[::downsample_factor]
            ts = wave.ts[::downsample_factor]
            effective_framerate = wave.framerate / downsample_factor
        else:
            ts = wave.ts
            effective_framerate = wave.framerate

        return Wave(envelope, ts, effective_framerate)

    def envelope_hilbert(self, wave: Wave, downsample_factor=None):
        """
        Envelope detection using Hilbert transform.
        This is the most accurate method but computationally more expensive.
        Now includes optional downsampling for improved efficiency.

        Args:
            wave: Input wave signal
            downsample_factor: Factor by which to downsample the envelope (1=no downsampling, 2=half rate, etc.)

        Returns:
            Wave object containing the envelope
        """
        if downsample_factor is None:
            downsample_factor = self.downsample_factor

        # Downsample input signal if requested (before Hilbert transform for efficiency)
        if downsample_factor > 1:
            # Apply anti-aliasing filter
            anti_alias_cutoff = wave.framerate / (2 * downsample_factor)
            anti_alias_sos = signal.butter(
                4, anti_alias_cutoff, 'low', fs=wave.framerate, output='sos')
            filtered_signal = signal.sosfilt(anti_alias_sos, wave.ys)

            # Downsample
            downsampled_signal = filtered_signal[::downsample_factor]
            effective_framerate = wave.framerate / downsample_factor
            ts = wave.ts[::downsample_factor]
        else:
            downsampled_signal = wave.ys
            effective_framerate = wave.framerate
            ts = wave.ts

        # Apply Hilbert transform
        analytic_signal = signal.hilbert(downsampled_signal)
        # Extract envelope (magnitude of analytic signal)
        envelope = np.abs(analytic_signal)

        return Wave(envelope, ts, effective_framerate)

    def envelope_rms(self, wave: Wave, window_size=1024, downsample_factor=None):
        """
        Envelope detection using RMS (Root Mean Square) over sliding window.
        Good for detecting energy changes in the signal.
        Now includes optional downsampling for improved efficiency.

        Args:
            wave: Input wave signal
            window_size: Size of the sliding window
            downsample_factor: Factor by which to downsample the envelope (1=no downsampling, 2=half rate, etc.)

        Returns:
            Wave object containing the envelope
        """
        if downsample_factor is None:
            downsample_factor = self.downsample_factor

        if downsample_factor > 1:
            # Downsample input signal first
            anti_alias_cutoff = wave.framerate / (2 * downsample_factor)
            anti_alias_sos = signal.butter(
                4, anti_alias_cutoff, 'low', fs=wave.framerate, output='sos')
            filtered_signal = signal.sosfilt(anti_alias_sos, wave.ys)
            downsampled_signal = filtered_signal[::downsample_factor]
            effective_framerate = wave.framerate / downsample_factor
            ts = wave.ts[::downsample_factor]
            # Adjust window size for downsampled signal
            window_size = max(1, window_size // downsample_factor)
        else:
            downsampled_signal = wave.ys
            effective_framerate = wave.framerate
            ts = wave.ts

        # Calculate RMS over sliding window
        envelope = []
        for i in range(len(downsampled_signal)):
            start_idx = max(0, i - window_size // 2)
            end_idx = min(len(downsampled_signal), i + window_size // 2)
            window = downsampled_signal[start_idx:end_idx]
            rms = np.sqrt(np.mean(window**2))
            envelope.append(rms)

        return Wave(np.array(envelope), ts, effective_framerate)

    def envelope_peak_decay(self, wave: Wave, attack_time=0.01, release_time=0.1, downsample_factor=None):
        """
        Envelope detection using peak detection with attack and release times.
        Good for real-time applications and mimics analog envelope followers.
        Now includes optional downsampling for improved efficiency.

        Args:
            wave: Input wave signal
            attack_time: Attack time constant (seconds)
            release_time: Release time constant (seconds)
            downsample_factor: Factor by which to downsample the envelope (1=no downsampling, 2=half rate, etc.)

        Returns:
            Wave object containing the envelope
        """
        if downsample_factor is None:
            downsample_factor = self.downsample_factor

        if downsample_factor > 1:
            # Downsample input signal first
            anti_alias_cutoff = wave.framerate / (2 * downsample_factor)
            anti_alias_sos = signal.butter(
                4, anti_alias_cutoff, 'low', fs=wave.framerate, output='sos')
            filtered_signal = signal.sosfilt(anti_alias_sos, wave.ys)
            downsampled_signal = filtered_signal[::downsample_factor]
            effective_framerate = wave.framerate / downsample_factor
            ts = wave.ts[::downsample_factor]
        else:
            downsampled_signal = wave.ys
            effective_framerate = wave.framerate
            ts = wave.ts

        # Convert time constants to sample-based coefficients
        attack_coeff = 1 - np.exp(-1 / (attack_time * effective_framerate))
        release_coeff = 1 - np.exp(-1 / (release_time * effective_framerate))

        envelope = np.zeros_like(downsampled_signal)
        current_envelope = 0

        for i, sample in enumerate(downsampled_signal):
            sample_abs = abs(sample)

            if sample_abs > current_envelope:
                # Attack phase
                current_envelope += attack_coeff * \
                    (sample_abs - current_envelope)
            else:
                # Release phase
                current_envelope += release_coeff * \
                    (sample_abs - current_envelope)

            envelope[i] = current_envelope

        return Wave(envelope, ts, effective_framerate)

    def envelope_peak_decay_realtime(self, sample, attack_time=0.005, release_time=0.05):
        """
        Real-time envelope detection optimized for beat detection in low-frequency signals.
        This method processes one sample at a time and is ideal for real-time beat detection.

        Args:
            sample: Current audio sample
            attack_time: Attack time constant (seconds) - fast for quick response
            release_time: Release time constant (seconds) - moderate for smooth decay

        Returns:
            Current envelope value
        """
        # Convert time constants to sample-based coefficients
        # Assuming 44.1kHz
        attack_coeff = 1 - np.exp(-1 / (attack_time * 44100))
        release_coeff = 1 - np.exp(-1 / (release_time * 44100))

        sample_abs = abs(sample)

        if sample_abs > self.current_envelope:
            # Attack phase - fast response to sudden increases
            self.current_envelope += attack_coeff * \
                (sample_abs - self.current_envelope)
        else:
            # Release phase - gradual decay
            self.current_envelope += release_coeff * \
                (sample_abs - self.current_envelope)

        return self.current_envelope

    def envelope_peak_decay_realtime_downsampled(self, sample, attack_time=0.005, release_time=0.05):
        """
        Downsampled version of real-time envelope detection.
        Only processes every downsample_factor samples for improved efficiency.

        Args:
            sample: Current audio sample
            attack_time: Attack time constant (seconds)
            release_time: Release time constant (seconds)

        Returns:
            Current envelope value (updated only every downsample_factor samples)
        """
        self.sample_counter += 1

        # Only process every downsample_factor samples
        if self.sample_counter % self.downsample_factor != 0:
            return -1  # self.current_envelope

        # Process the sample normally
        return self.envelope_peak_decay_realtime(sample, attack_time, release_time)

    def get_optimal_downsample_factor(self, wave: Wave, target_framerate=11025):
        """
        Calculate the optimal downsampling factor for a given target framerate.
        This helps balance efficiency with accuracy for peak detection.

        Args:
            wave: Input wave signal
            target_framerate: Target framerate for the envelope (default: 11025 Hz, good for beat detection)

        Returns:
            Optimal downsampling factor
        """
        if wave.framerate <= target_framerate:
            return 1

        factor = wave.framerate // target_framerate
        # Ensure factor is a power of 2 for efficient processing
        factor = 2 ** int(np.log2(factor))
        return max(1, factor)

    def envelope_for_peak_detection(self, wave: Wave, method='rectification', downsample_factor=None):
        """
        Optimized envelope detection specifically for peak detection applications.
        Automatically determines optimal downsampling factor if not specified.

        Args:
            wave: Input wave signal
            method: Envelope detection method ('rectification', 'hilbert', 'rms', 'peak_decay')
            downsample_factor: Downsampling factor (None for automatic optimization)

        Returns:
            Wave object containing the optimized envelope
        """
        # Auto-determine downsampling factor if not specified
        if downsample_factor is None:
            downsample_factor = self.get_optimal_downsample_factor(wave)

        # Choose the appropriate method
        if method == 'rectification':
            return self.envelope_rectification_lowpass(wave, downsample_factor=downsample_factor)
        elif method == 'hilbert':
            return self.envelope_hilbert(wave, downsample_factor=downsample_factor)
        elif method == 'rms':
            return self.envelope_rms(wave, downsample_factor=downsample_factor)
        elif method == 'peak_decay':
            return self.envelope_peak_decay(wave, downsample_factor=downsample_factor)
        else:
            raise ValueError(f"Unknown envelope method: {method}")

    def reset_downsampling_state(self):
        """Reset the downsampling state for real-time processing."""
        self.sample_counter = 0
        self.downsampled_buffer = []
        self.current_envelope = 0.0
