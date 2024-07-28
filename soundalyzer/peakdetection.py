import numpy as np
import time
from collections import deque


class PeakDetector:
    def __init__(self, attack=0.01, release=0.1, threshold_rel=0.1, min_peak_distance=0.3, sample_rate=44100):
        """
        Peak follower-based detector for smooth envelope data.
        Args:
            attack: Attack time in seconds (fast rise)
            release: Release time in seconds (slow fall)
            threshold_rel: Relative threshold above local min (fraction)
            min_peak_distance: Minimum time between peaks in seconds
            sample_rate: Sample rate in Hz
        """
        self.attack = attack
        self.release = release
        self.threshold_rel = threshold_rel
        self.min_peak_distance = min_peak_distance
        self.sample_rate = sample_rate
        self.min_peak_distance_samples = int(min_peak_distance * sample_rate)

        # Envelope follower state
        self.envelope = 0.0
        self.last_sample = 0.0
        self.peak_buffer = []
        self.processing_times = []
        
        # Ring buffers to prevent overflow issues
        self.local_min_buffer = deque(maxlen=int(0.5 * sample_rate))  # 0.5s window
        self.sample_times_buffer = deque(maxlen=int(1.0 * sample_rate))  # 1s window for time tracking
        self.envelope_buffer = deque(maxlen=int(0.1 * sample_rate))  # 0.1s window for envelope history
        
        # Peak detection state
        self.prev_env = 0.0
        self.is_rising = False
        self.peak_candidate = None
        self.last_peak_time = -min_peak_distance  # Use time instead of sample index
        
        # Performance tracking
        self.samples_processed = 0
        self.start_time = time.perf_counter()

    def _update_envelope(self, sample):
        sample_abs = abs(sample)
        if sample_abs > self.envelope:
            coeff = 1 - np.exp(-1 / (self.attack * self.sample_rate))
        else:
            coeff = 1 - np.exp(-1 / (self.release * self.sample_rate))
        self.envelope += coeff * (sample_abs - self.envelope)
        return self.envelope

    def _dynamic_threshold(self):
        # Use a moving minimum as the baseline
        if not self.local_min_buffer:
            return 0.0
        local_min = min(self.local_min_buffer)
        return local_min + self.threshold_rel * (max(self.local_min_buffer) - local_min)

    def add(self, sample, current_time=None, include_envelope=False):
        start_time = time.perf_counter()

        # Calculate current time if not provided
        if current_time is None:
            current_time = self.samples_processed / self.sample_rate

        # Update sample counter (will wrap around naturally, but we use time-based logic)
        self.samples_processed += 1

        # Update envelope
        if include_envelope:
            env = self._update_envelope(sample)
        else:
            env = sample

        # Update ring buffers
        self.local_min_buffer.append(env)
        self.sample_times_buffer.append(current_time)
        self.envelope_buffer.append(env)

        # Calculate dynamic threshold
        threshold = self._dynamic_threshold()

        # Peak detection: detect local maxima above threshold
        peak = None
        if env > threshold:
            if env > self.prev_env:
                self.is_rising = True
                self.peak_candidate = (current_time, env, threshold)
            elif env < self.prev_env and self.is_rising:
                # Local maximum - check time-based constraints
                if current_time - self.last_peak_time >= self.min_peak_distance and self.peak_candidate is not None:
                    peak = {
                        'index': self.samples_processed - 1,  # Use current sample count
                        'value': self.peak_candidate[1],
                        'time': self.peak_candidate[0],
                        'threshold': self.peak_candidate[2]
                    }
                    self.peak_buffer.append(peak)
                    self.last_peak_time = current_time
                self.is_rising = False
        else:
            self.is_rising = False

        self.prev_env = env
        self.last_sample = sample
        
        # Performance tracking
        processing_time = time.perf_counter() - start_time
        self.processing_times.append(processing_time)
        if len(self.processing_times) > 1000:
            self.processing_times = self.processing_times[-1000:]
            
        return peak

    def get_peaks(self):
        return self.peak_buffer

    def get_performance_stats(self):
        if not self.processing_times:
            return {}
        
        total_time = time.perf_counter() - self.start_time
        return {
            'avg_processing_time_ms': np.mean(self.processing_times) * 1000,
            'max_processing_time_ms': np.max(self.processing_times) * 1000,
            'min_processing_time_ms': np.min(self.processing_times) * 1000,
            'samples_processed': self.samples_processed,
            'total_processing_time_s': total_time,
            'samples_per_second': self.samples_processed / total_time if total_time > 0 else 0,
            'buffer_sizes': {
                'local_min_buffer': len(self.local_min_buffer),
                'sample_times_buffer': len(self.sample_times_buffer),
                'envelope_buffer': len(self.envelope_buffer)
            }
        }

    def reset(self):
        self.envelope = 0.0
        self.last_sample = 0.0
        self.peak_buffer = []
        self.samples_processed = 0
        self.processing_times = []
        self.start_time = time.perf_counter()
        
        # Clear all ring buffers
        self.local_min_buffer.clear()
        self.sample_times_buffer.clear()
        self.envelope_buffer.clear()
        
        # Reset peak detection state
        self.prev_env = 0.0
        self.is_rising = False
        self.peak_candidate = None
        self.last_peak_time = -self.min_peak_distance

    def set_min_peak_distance(self, min_distance_seconds):
        self.min_peak_distance = min_distance_seconds
        self.min_peak_distance_samples = int(min_distance_seconds * self.sample_rate)
        # Update last peak time if it's too far in the past
        if self.last_peak_time < -min_distance_seconds:
            self.last_peak_time = -min_distance_seconds

    def get_buffer_info(self):
        """Get information about the current state of ring buffers"""
        return {
            'local_min_buffer_size': len(self.local_min_buffer),
            'local_min_buffer_maxlen': self.local_min_buffer.maxlen,
            'sample_times_buffer_size': len(self.sample_times_buffer),
            'sample_times_buffer_maxlen': self.sample_times_buffer.maxlen,
            'envelope_buffer_size': len(self.envelope_buffer),
            'envelope_buffer_maxlen': self.envelope_buffer.maxlen,
            'samples_processed': self.samples_processed,
            'last_peak_time': self.last_peak_time
        }


def test_with_real_data():
    file_path = "/Users/tjabben/Documents/enveloped_samples.txt"
    try:
        with open(file_path, 'r') as f:
            samples = [float(line.strip()) for line in f if line.strip()]
        print(f"Loaded {len(samples)} samples from {file_path}")
        print(f"Sample range: {min(samples):.6f} to {max(samples):.6f}")
        non_zero_samples = [s for s in samples if s > 0.001]
        print(f"Non-zero samples (>0.001): {len(non_zero_samples)}")
        if non_zero_samples:
            print(f"Non-zero range: {min(non_zero_samples):.6f} to {max(non_zero_samples):.6f}")
        sample_rate = 44100
        detector = PeakDetector(
            attack=0.01, release=0.1, threshold_rel=0.1, min_peak_distance=0.4, sample_rate=sample_rate)
        detected_peaks = []
        for i, sample in enumerate(samples):
            peak = detector.add(sample, i / sample_rate)
            if peak:
                detected_peaks.append(peak)
                print(f"Peak: Index={peak['index']}, Time={peak['time']:.3f}s, Value={
                      peak['value']:.6f}, Threshold={peak['threshold']:.6f}")
        print(f"\nTotal peaks detected: {len(detected_peaks)}")
        print("Peak indices:", [p['index'] for p in detected_peaks])
        print("Peak values:", [p['value'] for p in detected_peaks])
        
        # Print performance stats
        stats = detector.get_performance_stats()
        print("\nPerformance Statistics:")
        for key, value in stats.items():
            if isinstance(value, dict):
                print(f"  {key}:")
                for subkey, subvalue in value.items():
                    print(f"    {subkey}: {subvalue}")
            else:
                print(f"  {key}: {value}")
        
        return detected_peaks
    except Exception as e:
        print(f"Error: {e}")
        return []


def test_overflow_scenario():
    """Test the peak detector with a large number of samples to verify overflow handling"""
    print("Testing overflow scenario...")
    sample_rate = 44100
    detector = PeakDetector(
        attack=0.01, release=0.1, threshold_rel=0.1, min_peak_distance=0.1, sample_rate=sample_rate)
    
    # Generate a large number of samples (more than 2^31 to test overflow)
    num_samples = 2500000000  # ~2.5 billion samples, ~15.7 hours at 44.1kHz
    
    print(f"Processing {num_samples:,} samples...")
    
    # Generate test signal with periodic peaks
    for i in range(min(num_samples, 1000000)):  # Limit for testing
        # Create a simple test signal with peaks every 10000 samples
        if i % 10000 == 0:
            sample = 0.8  # Peak
        elif i % 10000 < 100:
            sample = 0.1 + 0.1 * np.sin(i * 0.1)  # Decay
        else:
            sample = 0.01  # Background noise
            
        peak = detector.add(sample)
        if peak and i % 100000 == 0:  # Print every 100kth peak
            print(f"Peak at sample {peak['index']:,}, time {peak['time']:.3f}s")
    
    # Print final stats
    stats = detector.get_performance_stats()
    print(f"\nFinal stats after {stats['samples_processed']:,} samples:")
    print(f"  Total processing time: {stats['total_processing_time_s']:.2f}s")
    print(f"  Samples per second: {stats['samples_per_second']:.0f}")
    print(f"  Buffer info: {detector.get_buffer_info()}")


# Example usage and testing
if __name__ == "__main__":
    print("Testing real-time peak follower detection with envelope data...")
    print("=" * 60)
    test_with_real_data()
    
    print("\n" + "=" * 60)
    test_overflow_scenario()
