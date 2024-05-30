import numpy as np
import threading
from plot import MainWindow
from PyQt5 import QtCore, QtWidgets
import time


def soundalyzer_main():
    print(1)
    x = np.linspace(0, 10, 100)
    y = np.sin(x)
    main.plot(x, y)
    for i in range(1, 10):
        y = np.sin(x*i)
        main.update_plot(x, y)
        time.sleep(0.5)

if __name__ == "__main__":
    app = QtWidgets.QApplication([])
    main = MainWindow("Test")
    x = threading.Thread(target=soundalyzer_main)
    x.start()
    main.show()
    app.exec()

