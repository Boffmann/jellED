import numpy as np


class PeakDetector:
    def __init__(self, lag, threshold, influence):
        self.y = list()
        self.length = len(self.y)
        self.lag = lag
        self.threshold = threshold
        self.influence = influence
        self.signals = [0] * len(self.y)
        self.filteredY = np.array(self.y).tolist()
        self.avgFilter = [0] * len(self.y)
        self.stdFilter = [0] * len(self.y)

        max_bpm = 180.0
        self.min_time_between_beats_seconds = 60.0 / max_bpm
        self.last_beat_time = - self.min_time_between_beats_seconds # Do not prevent first beat from happening before time is passed

        # self.avgFilter[self.lag - 1] = np.mean(self.y[0:self.lag]).tolist()
        # self.stdFilter[self.lag - 1] = np.std(self.y[0:self.lag]).tolist()

    def thresholding_algo(self, new_value, sampling_time):
        self.y.append(new_value)
        i = len(self.y) - 1
        self.length = len(self.y)
        if i < self.lag:
            return 0
        elif i == self.lag:
            self.signals = [0] * len(self.y)
            self.filteredY = np.array(self.y).tolist()
            self.avgFilter = [0] * len(self.y)
            self.stdFilter = [0] * len(self.y)
            self.avgFilter[self.lag] = np.mean(self.y[0:self.lag]).tolist()
            self.stdFilter[self.lag] = np.std(self.y[0:self.lag]).tolist()
            return 0

        self.signals += [0]
        self.filteredY += [0]
        self.avgFilter += [0]
        self.stdFilter += [0]

        # print("V1: " + str(abs(self.y[i] - self.avgFilter[i - 1])) + " V2: " + str(self.threshold * self.stdFilter[i - 1]))
        # print("V1: " + str(self.avgFilter[i - 1]) + " V2: " + str(self.stdFilter[i - 1]))

        if abs(self.y[i] - self.avgFilter[i - 1]) > (self.threshold * self.stdFilter[i - 1]):

            if self.y[i] > self.avgFilter[i - 1] and sampling_time - self.last_beat_time > self.min_time_between_beats_seconds:
                self.last_beat_time = sampling_time
                self.signals[i] = 1
            else:
                self.signals[i] = 0 # -1

            self.filteredY[i] = self.influence * self.y[i] + \
                (1 - self.influence) * self.filteredY[i - 1]
            self.avgFilter[i] = np.mean(self.filteredY[(i - self.lag):i])
            self.stdFilter[i] = np.std(self.filteredY[(i - self.lag):i])
        else:
            self.signals[i] = 0
            self.filteredY[i] = self.y[i]
            self.avgFilter[i] = np.mean(self.filteredY[(i - self.lag):i])
            self.stdFilter[i] = np.std(self.filteredY[(i - self.lag):i])


        return self.signals[i]

