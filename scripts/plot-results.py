#!/usr/bin/env python3
import sys

if len(sys.argv) < 2:
    print('plot-results.py <benchmark-results>')
    sys.exit(1)

benchmarkResultsFileName = sys.argv[1];
print(benchmarkResultsFileName)
