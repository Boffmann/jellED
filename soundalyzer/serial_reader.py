import serial
import sys
import re
import datetime

class SerialReader:
    def __init__(self):
        port = "/dev/cu.usbserial-0001"
        baudrate = 115200
        self.ser = serial.Serial(port, baudrate)
        self.rex = r"b'(-?[0-9].[0-9]+)\\r\\n'"


    def read_samples(self, time_span_seconds):
        # Somehow the first ~3 seconds of the input are blurry.
        # It looks like the ESP gets ready for Serial transfer somehow and
        # outputs random data. Therefore, the first two seconds are dropped
        started = datetime.datetime.now()
        while (datetime.datetime.now() - started).seconds < 3:
            self.ser.readline()

        result = []
        started = datetime.datetime.now()
        now = started
        while (datetime.datetime.now() - started).seconds < time_span_seconds:
            line = self.ser.readline()
            match = re.match(self.rex, str(line))
            if match is not None:
                result.append(float(match.group(1)))

        return result

