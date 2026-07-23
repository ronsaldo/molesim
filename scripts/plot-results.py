#!/usr/bin/env python3
import sys
import matplotlib.pyplot as plt
import numpy as np

if len(sys.argv) < 2:
    print('plot-results.py <benchmark-results>')
    sys.exit(1)

class Measurements:
    def __init__(self, algorithmName, averages, stds):
        self.algorithmName = algorithmName
        self.averages = averages
        self.stds = stds

    def plot(self, Xs):
        plt.errorbar(Xs, self.averages, self.stds, label=self.algorithmName)

def parseAverageAndStd(averageLine, stdLine):
    averageLineComponents = averageLine.split(',')
    stdLineComponents = stdLine.split(',')
    algorithmName = averageLineComponents[0]
    averages = np.array(list(map(lambda x: float(x), averageLineComponents[1:])))
    stds = np.array(list(map(lambda x: float(x), stdLineComponents[1:])))
    return Measurements(algorithmName, averages, stds);

fig = plt.figure()
benchmarkResultsFileName = sys.argv[1];
with open(benchmarkResultsFileName, 'r') as f:
    lines = f.read().splitlines()
    Ns = np.array(list(map(lambda x: int(x), lines[0].split(',')[1:])))
    naiveMeasurements = parseAverageAndStd(lines[1], lines[2])
    gridMeasurements = parseAverageAndStd(lines[3], lines[4])
    kdTreeMeasurements = parseAverageAndStd(lines[5], lines[6])
    octreeMeasurements = parseAverageAndStd(lines[7], lines[8])
    bvhMeasurements = parseAverageAndStd(lines[9], lines[10])

    naiveMeasurements.plot(Ns)
    gridMeasurements.plot(Ns)
    kdTreeMeasurements.plot(Ns)
    octreeMeasurements.plot(Ns)
    bvhMeasurements.plot(Ns)

plt.legend()
plt.show()