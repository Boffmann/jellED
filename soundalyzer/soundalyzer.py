import numpy as np
import threading
from mainwindow import MainWindow
from wav import Wave, read_wave
import iir
from PyQt5 import QtCore, QtWidgets
import time


def soundalyzer_main():
    # wave = read_wave("/Users/tjabben/Documents/techno-drums-loop-120-bpm-1-131243.wav")
    wave = read_wave("/Users/tjabben/Documents/techno-drums-loop-120-bpm-monno.wav")
    # wave = read_wave("/Users/tjabben/repos/jellED/soundalyzer/sound.wav")
    main.plot(plot_index, wave.ts, wave.ys)
    filtered_wave = iir.filter_lowpass_scipy(wave)
    main.plot(plot_index_2, filtered_wave.ts, filtered_wave.ys)
    filtered_own = iir.filter_lowpass_own(wave)
    main.plot(plot_index_3, filtered_own.ts, filtered_own.ys)
    main.start_playing_wave()
    filtered_own.play()
    # wave.play()

if __name__ == "__main__":
    app = QtWidgets.QApplication([])
    main = MainWindow()
    plot_index = main.add_wave_plot("Original")
    plot_index_2 = main.add_wave_plot("Filtered via Scipy")
    plot_index_3 = main.add_wave_plot("Filtered With own algo")

    x = threading.Thread(target=soundalyzer_main)
    x.start()
    # soundalyzer_main()
    main.show()
    app.exec()

