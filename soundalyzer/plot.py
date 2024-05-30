import pyqtgraph as pg
from random import randint
from PyQt5 import QtCore, QtWidgets

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
        self.plot_graph.setYRange(-2, 2)
        self.plot_graph.setXRange(0, 110)

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
