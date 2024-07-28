import serial
import sys
import re

class SerialReader:
    def __init__(self):
        port = "/dev/cu.usbserial-0001"
        baudrate = 115200
        self.ser = serial.Serial(port, baudrate)
        self.rex = r"b'(-?[0-9].[0-9]+)\\r\\n'"

    def read_samples(self, framerate, time_span_seconds):
        total_expected_samples = framerate * time_span_seconds
        samples_read = 0
        result = []
        while samples_read < total_expected_samples:
            line = self.ser.readline()
            match = re.match(self.rex, str(line))
            if match is not None:
                samples_read += 1
                result.append(float(match.group(1)))

        return result

