import numpy as np
import sys

arr = np.load(sys.argv[1])

i = 0
for e in arr.tolist():
    if e != 'Element ' + str(i):
        exit(1)
    i += 1

