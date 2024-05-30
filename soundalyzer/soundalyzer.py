import numpy as np
import threading
from plot import MainWindow
from wav import Wave, read_wave
from PyQt5 import QtCore, QtWidgets
import time


def soundalyzer_main():
    wave = read_wave("/Users/tjabben/Documents/techno-drums-loop-120-bpm-1-131243.wav")
    main.plot(wave.ts, wave.ys)

if __name__ == "__main__":
    app = QtWidgets.QApplication([])
    main = MainWindow("Test")
    x = threading.Thread(target=soundalyzer_main)
    x.start()
    main.show()
    app.exec()

