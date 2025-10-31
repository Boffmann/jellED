import numpy as np
import argparse
import scipy.signal as signal
import threading
from mainwindow import MainWindow
from wav import Wave, WavFileWriter, wave_from_wav, wave_from_recorded_file
from bandpass import BandpassFilter
from envelope import EnvelopeDetector
from peakdetection import PeakDetector
from serialreader import SerialReader
import filter
from PyQt5 import QtWidgets
from pathlib import Path
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
        fileWriter = WavFileWriter(
            filename="output.wav", framerate=wave.framerate)
        fileWriter.write(wave)
        fileWriter.close()
    return wave


def generate_butterworth():
    order = 4
    lowcut = 50
    highcut = 200
    framerate = 48000
    sos = signal.butter(order, [lowcut, highcut],
                        'band', fs=framerate, output='sos')
    for o in range(0, order):
        for i in range(0, 3):
            print("this->numerator[{order}][{place}] = {result};".format(
                order=o, place=i % 3, result=sos[o][i]))
        for i in range(3, 6):
            print("this->denominator[{order}][{place}] = {result};".format(
                order=o, place=i % 3, result=sos[o][i]))


def generate_test_expectations():
    working_dir = str(Path.cwd())
    Path(working_dir + "/../jellED/core/test/beatDetection/resources").mkdir(parents=True, exist_ok=True)
    original_sample_file = working_dir + "/../jellED/core/test/beatDetection/resources/unfiltered_test_values.txt"
    filtered_sample_file = working_dir + "/../jellED/core/test/beatDetection/resources/filtered_test_values.txt"
    envelope_sample_file = working_dir + "/../jellED/core/test/beatDetection/resources/enveloped_test_values.txt"
    peaks_sample_file = working_dir + "/../jellED/core/test/beatDetection/resources/peak_values.txt"

    order = 4
    lowcut = 50
    highcut = 100
    test_buffer_length = 100000

    envelope_size = 1024
    downsampling_factor = 8
    envelopeDetector_downsampled = EnvelopeDetector(
        envelope_size, envelope_size / 2, downsampling_factor)
    envelopeDetector_orin = EnvelopeDetector(
        envelope_size, envelope_size / 2, 1)

    wave = wave_from_wav(
        "/Users/tjabben/Documents/techno-drums-loop-120-bpm-1-44100.wav")
    # smaller_wave = wave
    smaller_wave = Wave(
        ys=wave.ys[:test_buffer_length], framerate=wave.framerate)

    peak_lag = 2048
    peak_threshold = 1.2
    peak_influence = 0.5
    peakDetector = PeakDetector(
        0.01, 0.1, 0.1, 0.4, smaller_wave.framerate / downsampling_factor)

    print("--generating unfiltered--")
    main.plot(plot_index, smaller_wave.ts, smaller_wave.ys)
    print("--generating filtered--")
    filtered = filter.filter_bandpass_scipy(
        smaller_wave, lowcut=lowcut, highcut=highcut, order=order)
    main.plot(plot_index_2, filtered.ts, filtered.ys)
    print("--generating envelope--")
    enveloped_original = []
    enveloped_downsampled = []
    peaks = (list(), list())

    for filtered_sample_index in range(len(filtered.ys)):
        filtered_sample = filtered.ys[filtered_sample_index]
        enveloped_sample = envelopeDetector_downsampled.envelope_peak_decay_realtime_downsampled(
            filtered_sample)
        if enveloped_sample != -1:
            enveloped_downsampled.append(enveloped_sample)
        enveloped_sample = envelopeDetector_orin.envelope_peak_decay_realtime_downsampled(
            filtered_sample)
        enveloped_original.append(enveloped_sample)

    downsampled_ts = calculate_ts(
        0, smaller_wave.ts[-1], smaller_wave.ts[-1] / len(enveloped_downsampled), len(enveloped_downsampled))
    main.plot(plot_index_3, downsampled_ts, enveloped_downsampled)
    main.plot(plot_index_3, filtered.ts, enveloped_original, color=(0, 255, 0))
    print(len(enveloped_downsampled))

    for ts_index in range(len(enveloped_downsampled)):
        envelope_sample = enveloped_downsampled[ts_index]
        # ts = downsampled_ts[ts_index]
        is_peak = 0 if peakDetector.add(envelope_sample, include_envelope=True) is None else 1
        if is_peak:
            peaks[0].append(ts_index)
        peaks[1].append(is_peak)
    main.plot(plot_index_3, downsampled_ts, peaks[1], color=(255, 0, 255))
    # return
    with open(peaks_sample_file, "w") as peaks_file:
        for peak in peaks[0]:
            peaks_file.write(str(peak) + "\n")
    np.savetxt(envelope_sample_file, enveloped_downsampled, fmt='%f')
    np.savetxt(filtered_sample_file, filtered.ys, fmt='%f')
    np.savetxt(original_sample_file, smaller_wave.ys, fmt='%f')


def soundalyzer_main(mode):
    global plot_index
    global plot_index_2
    global plot_index_3
    global main
    peaks = []

    if mode == "WAV":
        wave = wave_from_wav(
            "/Users/tjabben/Documents/techno-drums-loop-120-bpm-1-44100.wav")
        # with open("/Users/tjabben/repos/jellED/jellED/raspi/build/samples.txt", mode="r") as peak_file:
        #     for line in peak_file.readlines():
        #         peaks.append(float(line.strip('\n')))
        # wave = wave_from_wav("/Users/tjabben/repos/jellED/soundalyzer/output.wav")
        # bandpass = BandpassFilter(4, 20, 100, wave.framerate)
        # Experiment: Bass übersteuert das Mikro komplett. Kann man Beats durch einen Workaround besser an den hohen Frquenzen erkennen?

        bandpass = BandpassFilter(4, 50, 100, wave.framerate)
        # bandpass.activate_downsampling(8000)
    elif mode == "RECORDED":
        # wave = wave_from_recorded_file("/Users/tjabben/repos/jellED/jellED/raspi/build/outfile.txt", 44100, 2, 2)
        wave = wave_from_recorded_file(
            "/Users/tjabben/repos/personal/jellED/jellED/raspi/build/debug_audio_samples.txt", 48000, 5, 0)
        wave.normalize()
        print(len(wave.ys))
        # fileWriter = WavFileWriter(filename="output.wav", framerate=wave.framerate)
        # fileWriter.write(wave)
        # fileWriter.close()
        bandpass = BandpassFilter(4, 20, 100, wave.framerate)
    elif mode == "RECORD":
        wave = read_from_serial(False)
        bandpass = BandpassFilter(4, 20, 100, wave.framerate)
    elif mode == "RECORD_STORE":
        wave = read_from_serial(True)
        bandpass = BandpassFilter(4, 2500, 3500, wave.framerate)
    elif mode == "GEN_BUTTERWORTH":
        generate_butterworth()
        return
    elif mode == "GEN_TEST_EXPECTATIONS":
        generate_test_expectations()
        return
    else:
        print("Mode " + mode + " is not known")
        return

    main.plot(plot_index, wave.ts, wave.ys)
    envelope_size = 1024
    envelopeDetector = EnvelopeDetector(envelope_size, envelope_size / 2)
    peakDetector = PeakDetector(16, 3, 0.3)

    bandpass_filtered = []
    envelope = []
    now = datetime.datetime.now()
    prev_env_value = -1
    first = True
    for index in range(len(wave.ys)):
        sample = wave.ys[index]
        ts = wave.ts[index]
        filtered_sample = bandpass.iir_filter_sos(sample)
        if filtered_sample != None:
            bandpass_filtered.append(filtered_sample)
            enveloped_sample = envelopeDetector.envelope_peak_decay_realtime(
                filtered_sample)
            if prev_env_value != enveloped_sample or first:
                prev_env_value = enveloped_sample
                envelope.append(enveloped_sample)
                first = False
                # is_peak = peakDetector.thresholding_algo(enveloped_sample, ts)
                # peaks.append(is_peak)
    print("Calculating with custom implementation took: " +
          str(datetime.datetime.now() - now))
    downsampled_ts = calculate_ts(
        0, wave.ts[-1], wave.ts[-1] / len(envelope), len(envelope))
    main.plot(plot_index_2, wave.ts, bandpass_filtered)
    main.plot(plot_index_3, downsampled_ts, envelope, color=(0, 0, 255))
    # main.plot(plot_index_3, wave_ts, wave_ys, color=(0, 255, 0))
    # main.plot(plot_index_3, downsampled_ts, envelope, color=(0, 255, 0))
    np.savetxt('/Users/tjabben/Documents/envelope.txt', envelope, fmt='%f')
    downsampled_peak_ts = calculate_ts(
        0, wave.ts[-1], wave.ts[-1] / len(peaks), len(peaks))
    # main.plot(plot_index_3, downsampled_peak_ts, peaks, color=(255, 0, 255))

    # print(len(bandpass_filtered))
    # return
    # filtered_own = Wave(bandpass_filtered, downsampled_ts, 8000)
    main.start_playing_wave()
    # filtered_own.play()
    wave.play()


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
    parser.add_argument(
        "--mode", help="Run Soundalyzer with mode. Select one of ('WAV'|'RECORDED'|RECORD|RECORD_STORE)", required=True)
    parser.add_argument(
        "--explain", help="Show more detailled help text with mode description", action="store_true")
    parser.add_argument("--no_gui", help="Do not show gui",
                        action="store_true")

    args = parser.parse_args()

    run(args)
