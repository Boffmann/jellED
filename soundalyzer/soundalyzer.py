import numpy as np
import threading
from mainwindow import MainWindow
from wav import Wave, read_wave
import math
import iir
from PyQt5 import QtCore, QtWidgets
import time
import datetime

def filter_envelope(wave: Wave):
    frame_size = 1024
    hop_length = 512
    result = np.array([max(wave.ys[i:i+frame_size]) for i in range(0, wave.ys.size, hop_length)])
    print("Result length: " + str(len(result)))
    ts = np.arange(0, wave.ts[-1], wave.ts[-1] / len(result))
    return ts, result

def filter_envelope_backward(wave: Wave):
    frame_size = 1024
    hop_length = 512

    result = []
    for i in range(0, wave.ys.size, hop_length):
        sample = wave.ys[i]
        window_max = 0
        for j in range(frame_size):
            backward_sample = 0 if i-j < 0 else wave.ys[i-j]
            window_max = max(window_max, backward_sample)
        result.append(window_max)

    print("Result length: " + str(len(result)))
    ts = np.arange(0, wave.ts[-1], wave.ts[-1] / len(result))
    return ts, result

def soundalyzer_main():
    wave = read_wave("/Users/tjabben/Documents/techno-drums-loop-120-bpm-monno.wav")
    main.plot(plot_index, wave.ts, wave.ys, color=(0, 0, 0))
    filtered_own = iir.filter_bandpass_own(wave)
    main.plot(plot_index, filtered_own.ts, filtered_own.ys, color=(0, 255, 0))
    now = datetime.datetime.now()
    ts, lmax = filter_envelope(filtered_own)
    print("Envelope calculation took: " + str(datetime.datetime.now() - now))
    main.plot(plot_index_2, ts, lmax, color=(255, 0, 0))
    ts, lmax = filter_envelope_backward(filtered_own)
    main.plot(plot_index_2, ts, lmax, color=(0, 255, 0))
    # main.plot(plot_index, ts, lmax, color=(255, 0, 0))
    # lowpassed = iir.filter_lowpass_fir(filtered_own)
    # main.plot(plot_index, lowpassed.ts, lowpassed.ys, color=(255, 0, 0))
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

