import pandas as pd
from matplotlib import pyplot as plt
from more_itertools import chunked
import getopt, sys

arguments = sys.argv[1:]
Smoothly = False
Smoothly_Interval = 5

short_opts = 'f:s:'
long_opts = ['file', 'smoothly']

try:
    options, values = getopt.getopt(arguments, short_opts, long_opts)
    for currentArgument, currentValue in options:
        if currentArgument in ('-f', '--file'):
            file = currentValue
        if currentArgument in ('-s', '--smoothly'):
            Smoothly = True
            Smoothly_Interval = int(currentValue)
except getopt.error as err:
    print(str(err))
    sys.exit(2)

data = pd.read_csv(file)

time = data['time']
tt_util = data['total_util']
gc_util = data['gc_util']

# Smoothly handle the data
if Smoothly:
    tt_util = [sum(x)/len(x) for x in chunked(tt_util, Smoothly_Interval)]
    gc_util = [sum(x)/len(x) for x in chunked(gc_util, Smoothly_Interval)]
    length = len(tt_util)
    time = time[:length]
    for i in range(0, len(time)):
        time[i] *= Smoothly_Interval

plt.plot(time, tt_util, color='blue', label='total_util')
plt.plot(time, gc_util, color='red', label='gc_util')
plt.legend()
plt.show()
