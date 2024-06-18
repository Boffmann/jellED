import numpy as np
import argparse
import scipy.signal as signal
import threading
from mainwindow import MainWindow
from wav import Wave, WavFileWriter, wave_from_wav, wave_from_recorded_file
import wave
from bandpass import BandpassFilter
from envelope import EnvelopeDetector
from peakdetection import PeakDetector
from serialreader import SerialReader
import math
import filter
from PyQt5 import QtCore, QtWidgets
import time
import datetime

plot_index = None
plot_index_2 = None
plot_index_3 = None
main = None

def calculate_ts(start, end, step_size, desired_length):
    ts = np.arange(start, end, step_size)
    ys_ts_delta = desired_length - len(ts)
    if ys_ts_delta < 0:
        ts = ts[:len(ts) - abs(ys_ts_delta)]
    elif ys_ts_delta > 0:
        for i in range(ys_ts_delta):
            ts.append(ts[-1] + step_size)
    return ts

def read_from_serial(should_store):
    serialReader = SerialReader()

    readTimeSpan = 2
    framerate = 8000
    serialData = serialReader.read_samples(framerate, readTimeSpan)
    print(serialData[0:20])
    print(len(serialData))
    step_size = readTimeSpan / len(serialData)
    ts = calculate_ts(0, readTimeSpan, step_size, len(serialData))
    wave = Wave(serialData, ts=ts, framerate=framerate)
    if should_store:
        fileWriter = WavFileWriter(filename="output.wav", framerate=wave.framerate)
        fileWriter.write(wave)
        fileWriter.close()
    return wave

def soundalyzer_main(mode):

    global plot_index
    global plot_index_2
    global plot_index_3
    global main

    if mode == "WAV":
        # wave = wave_from_wav("/Users/tjabben/Documents/techno-drums-loop-120-bpm-monno.wav")
        wave = wave_from_wav("/Users/tjabben/repos/jellED/soundalyzer/output.wav")
        # bandpass = BandpassFilter(4, 20, 100, wave.framerate)
        # Experiment: Bass übersteuert das Mikro komplett. Kann man Beats durch einen Workaround besser an den hohen Frquenzen erkennen?
        bandpass = BandpassFilter(4, 2999, 3999, wave.framerate)
        bandpass.activate_downsampling(8000)
    elif mode == "RECORDED":
        # wave = wave_from_recorded_file("/Users/tjabben/repos/jellED/jellED/soundinput.txt", 8000, 2, 10)
        wave = wave_from_recorded_file("/Users/tjabben/repos/jellED/jellED/soundinput.txt", 8000, 2, 10)
        bandpass = BandpassFilter(4, 20, 100, wave.framerate)
    elif mode == "RECORD":
        wave = read_from_serial(False)
        bandpass = BandpassFilter(4, 20, 100, wave.framerate)
    elif mode == "RECORD_STORE":
        wave = read_from_serial(True)
        bandpass = BandpassFilter(4, 2500, 3500, wave.framerate)
    else:
        print("Mode " + mode + " is not known")
        return


    main.plot(plot_index, wave.ts, wave.ys)
    envelope_size = 128
    envelopeDetector = EnvelopeDetector(envelope_size, envelope_size / 2)
    peakDetector = PeakDetector(2048, 3, 0.3)

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
            # prev_env_value = enveloped_sample
            envelope.append(enveloped_sample)
            # first = False
            is_peak = peakDetector.thresholding_algo(enveloped_sample, ts)
            peaks.append(is_peak)
    print("Calculating with custom implementation took: " + str(datetime.datetime.now() - now))
    downsampled_ts = calculate_ts(0, wave.ts[-1], wave.ts[-1] / len(bandpass_filtered), len(bandpass_filtered))
    main.plot(plot_index_2, downsampled_ts, bandpass_filtered)
    main.plot(plot_index_2, downsampled_ts, envelope, color=(0, 0, 255))
    # main.plot(plot_index_3, wave_ts, wave_ys, color=(0, 255, 0))
    main.plot(plot_index_3, downsampled_ts, envelope, color=(0, 255, 0))
    main.plot(plot_index_3, downsampled_ts, peaks, color=(255, 0, 255))

    # print(len(bandpass_filtered))
    filtered_own = Wave(bandpass_filtered, downsampled_ts, 8000)
    main.start_playing_wave()
    filtered_own.play()
    # wave.play()

def show_help():
    print("Mode WAV: Reads a Wave from a wav file and performs filtering, envelope, and peak detection")
    print("Mode RECORDED: Reads a Wave from a recorded file that was read from the ESP prior execution. Performs filtering, envelope, and peak detection")
    print("Mode RECORD: Records Sample from connected ESP. Performs filtering, envelope, and peak detection on the recorded sample")
    print("Mode RECORD_STORE: Records Sample from connected ESP and stores it in output file. Performs filtering, envelope, and peak detection on the recorded sample")

def run(args):
    global plot_index
    global plot_index_2
    global plot_index_3
    global main
    if args.explain:
        show_help()
        return

    if args.no_gui:
        soundalyzer_main(args.mode)
        return

    app = QtWidgets.QApplication([])
    main = MainWindow()
    plot_index = main.add_wave_plot("1")
    plot_index_2 = main.add_wave_plot("2")
    plot_index_3 = main.add_wave_plot("3")

    x = threading.Thread(target=soundalyzer_main, args=(args.mode,))
    x.start()
    # soundalyzer_main()
    main.show()
    app.exec()

if __name__ == "__main__":
    parser = argparse.ArgumentParser("soundalyzer")
    parser.add_argument("--mode", help="Run Soundalyzer with mode. Select one of ('WAV'|'RECORDED'|RECORD|RECORD_STORE)", required=True)
    parser.add_argument("--explain", help="Show more detailled help text with mode description", action="store_true")
    parser.add_argument("--no_gui", help="Do not show gui", action="store_true")

    args = parser.parse_args()

    run(args)

