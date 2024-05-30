import pyqtgraph as pg
from random import randint
from PyQt5 import QtCore, QtWidgets
import datetime

class MainWindow(QtWidgets.QMainWindow):
    def __init__(self, title):
        super().__init__()

        self.plot_graph = pg.PlotWidget()
        self.setCentralWidget(self.plot_graph)
        self.plot_graph.setBackground("w")
        self.pen = pg.mkPen(color=(255, 0, 0))
        self.plot_graph.setTitle(title, color="b", size="20pt")
        styles = {"color": "red", "font-size": "18px"}
        self.plot_graph.setLabel("left", "Y", **styles)
        self.plot_graph.setLabel("bottom", "Time (min)", **styles)
        self.plot_graph.addLegend()
        self.plot_graph.showGrid(x=True, y=True)
        self.plot_graph.setYRange(-1, 1)
        self.plot_graph.setXRange(0, 17)
        self.begin_vLine = pg.InfiniteLine(angle=90, movable=False, pen=pg.mkPen('g', width=4, style=QtCore.Qt.SolidLine))
        self.plot_graph.addItem(self.begin_vLine, ignoreBounds=True)

        self.start_time = None
        self.timer = QtCore.QTimer()
        self.timer.setInterval(15)
        self.timer.timeout.connect(self.update_plot_periodically)
        self.timer.start()

    def plot(self, x, y):
        print("Plot")
        self.line = self.plot_graph.plot(
            x,
            y,
            name="Temperature Sensor",
            pen=self.pen,
        )


    def update_plot(self, x, y):
        self.line.setData(x, y)

    def start_playing_wave(self):
        self.start_time = datetime.datetime.now() + datetime.timedelta(microseconds=200000)

    def update_plot_periodically(self):
        if self.start_time == None:
            print("Not Started yet")
            return
        time_passed = datetime.datetime.now() - self.start_time
        self.begin_vLine.pos = time_passed.seconds
        # self.begin_vLine.setValue((time_passed.seconds + ((float)time_passed.microseconds / 1000000.0)))
        self.begin_vLine.setValue(float(time_passed.seconds) + (float(time_passed.microseconds) / 1000000.0))
        # print (float(time_passed.seconds) + (float(time_passed.microseconds) / 1000000.0))

