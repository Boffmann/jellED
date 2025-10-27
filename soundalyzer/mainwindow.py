import pyqtgraph as pg
from random import randint
from PyQt5 import QtCore, QtWidgets
import datetime
from multiprocessing import Queue

class WavePlot:
    def __init__(self, title):
        self.plot_graph = pg.PlotWidget()
        self.plot_graph.setBackground("w")
        self.plot_graph.setTitle(title, color="b", size="20pt")
        styles = {"color": "red", "font-size": "18px"}
        self.plot_graph.setLabel("left", "Y", **styles)
        # self.plot_graph.setLabel("bottom", "Time (min)", **styles)
        self.plot_graph.addLegend()
        self.plot_graph.showGrid(x=True, y=True)
        self.plot_graph.setYRange(-1, 1)
        self.plot_graph.setXRange(0, 17)
        # self.plot_graph.setXRange(0.25, 0.35)
        self.begin_vLine = pg.InfiniteLine(angle=90, movable=False, pen=pg.mkPen('g', width=4, style=QtCore.Qt.SolidLine))
        self.plot_graph.addItem(self.begin_vLine, ignoreBounds=True)

        # self.commands = Queue()

        self.start_time = None
        self.timer = QtCore.QTimer()
        self.timer.setInterval(15)
        self.timer.timeout.connect(self.process_command_queue)
        self.timer.start()

    def plot(self, x, y, color):
        pen = pg.mkPen(color=color)
        self.line = self.plot_graph.plot(
            x,
            y,
            pen=pen,
        )

    def get_plot_graph(self):
        return self.plot_graph

    def start_playing_wave(self):
        print("Start playint")
        self.start_time = datetime.datetime.now() + datetime.timedelta(microseconds=200000)

    def process_command_queue(self):
        if self.start_time == None:
            return
        time_passed = datetime.datetime.now() - self.start_time
        self.begin_vLine.pos = time_passed.seconds
        self.begin_vLine.setValue(float(time_passed.seconds) + (float(time_passed.microseconds) / 1000000.0))

    # def process_command_queue(self):
    #     time_start = datetime.datetime.now()
    #     time_run = 0
    #     while time_run < 14:
    #         command = self.commands.get()


class MainWindow(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.h_layout = QtWidgets.QVBoxLayout()
        centralWidget = QtWidgets.QWidget()
        centralWidget.setLayout(self.h_layout)
        self.setCentralWidget(centralWidget)

        self.resize(2000, 1000)

        self.plots = []


    def add_wave_plot(self, title):
        """
            Adds a new wave plot and returns its index
        """
        plot = WavePlot(title)
        self.plots.append(plot)
        self.h_layout.addWidget(plot.get_plot_graph())
        return len(self.plots) - 1

    def plot(self, plot_index, x, y, color=(255, 0, 0)):
        if plot_index >= len(self.plots):
            print(f"Plot index {plot_index} is larger than plot length {len(self.plots)}")
            return
        self.plots[plot_index].plot(x,y, color)

    def update_plot(self, x, y):
        self.line.setData(x, y)

    def start_playing_wave(self):
        for plot in self.plots:
            plot.start_playing_wave()

