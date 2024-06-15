import numpy as np
import scipy.signal as signal
import threading
from mainwindow import MainWindow
from wav import Wave, read_wave
import wave
from bandpass import BandpassFilter
from envelope import EnvelopeDetector
from peakdetection import PeakDetector
from serial_reader import SerialReader
import math
import filter
from PyQt5 import QtCore, QtWidgets
import time
import datetime

def write_wav_file(data):
    with wave.open("output.wav", mode="wb") as wav_file:
        sampwidth = 2
        wav_file.setnchannels(1)
        wav_file.setsampwidth(sampwidth)
        wav_file.setframerate(len(data) / 10)
        # output = np.array(wa)
        bits = sampwidth * 8
        bound = 2 ** (bits - 1) - 1
        ys = np.array(data)
        zs = (ys * bound).astype(np.int16)
        wav_file.writeframes(zs.tobytes())
        # wav_file.writeframes(bytes(int(data * 32768)))

def soundalyzer_main():
    wave = read_wave("/Users/tjabben/Documents/techno-drums-loop-120-bpm-monno.wav")
    main.plot(plot_index, wave.ts, wave.ys, color=(0, 0, 0))
    bandpass = BandpassFilter(4, 20, 100, wave.framerate)
    bandpass.activate_downsampling(wave.framerate, 1000)
    envelopeDetector = EnvelopeDetector(32, 16)
    peakDetector = PeakDetector(256, 3, 0.3)
    serialReader = SerialReader()

    # REGION: Read data samples from serial port

    readTimeSpan = 10
    serialData = serialReader.read_samples(readTimeSpan)
    print(serialData[0:20])
    print(len(serialData))
    step_size = readTimeSpan / len(serialData)
    ts = np.arange(0, readTimeSpan, step_size)
    ys_ts_delta = len(serialData) - len(ts)
    if ys_ts_delta < 0:
        ts = ts[:len(ts) - abs(ys_ts_delta)]
    elif ys_ts_delta > 0:
        for i in range(ys_ts_delta):
            ts.append(ts[-1] + step_size)
    main.plot(plot_index_2, ts, serialData)
    write_wav_file(serialData)

    # END REGION: Read data samples from serial port
    return

    print("Wave FPS: " + str(wave.framerate))
    bandpass_filtered = []
    envelope = []
    peaks = []
    now = datetime.datetime.now()
    prev_env_value = -1
    first = True
    for index in range(len(wave.ys)):
        sample = wave.ys[index]
        ts = wave.ts[index]
        filtered_sample = bandpass.iir_filter_sos(sample)
        if filtered_sample != None:
            bandpass_filtered.append(filtered_sample)
            enveloped_sample = envelopeDetector.envelope_sample(filtered_sample)
            # if prev_env_value != enveloped_sample or first:
            prev_env_value = enveloped_sample
            envelope.append(enveloped_sample)
            first = False
            is_peak = peakDetector.thresholding_algo(enveloped_sample, ts)
            peaks.append(is_peak)

    # print("Length of filtered: " + str(len(bandpass_filtered)))
    print("Calculating with custom implementation took: " + str(datetime.datetime.now() - now))
    ts = np.arange(0, wave.ts[-1], wave.ts[-1] / len(bandpass_filtered))
    main.plot(plot_index_2, ts, bandpass_filtered)
    # main.plot(plot_index_2, wave.ts, envelope, color=(0, 0, 255))
    # main.plot(plot_index_3, wave.ts, envelope, color=(0, 255, 0))
    # main.plot(plot_index_3, wave.ts, peaks, color=(255, 0, 255))
    main.plot(plot_index_2, ts, envelope, color=(0, 0, 255))
    main.plot(plot_index_3, ts, envelope, color=(0, 255, 0))
    main.plot(plot_index_3, ts, peaks, color=(255, 0, 255))

    # now = datetime.datetime.now()
    # filtered = filter.filter_bandpass(wave, lowcut=20, highcut=100, order=4)
    # ts, ys = envelopeDetector.filter_envelope_backward(filtered)
    # print("Calculating with scipy+numpy took: " + str(datetime.datetime.now() - now))
    # main.plot(plot_index_3, filtered.ts, filtered.ys, color=(0, 0, 255))
    # main.plot(plot_index_3, ts, ys, color=(255, 0, 0))

    # main.start_playing_wave()
    # filtered_own.play()
    # wave.play()

if __name__ == "__main__":
    app = QtWidgets.QApplication([])
    main = MainWindow()
    plot_index = main.add_wave_plot("1")
    plot_index_2 = main.add_wave_plot("2")
    plot_index_3 = main.add_wave_plot("3")

    x = threading.Thread(target=soundalyzer_main)
    x.start()
    # soundalyzer_main()
    main.show()
    app.exec()

