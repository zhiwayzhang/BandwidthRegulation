import pandas as pd
from matplotlib import pyplot as plt
from more_itertools import chunked

data = pd.read_csv('data.csv')

Smothly = True
Smothly_Interval = 5

time = data['time']
tt_util = data['total_util']
gc_util = data['gc_util']

# smothly handle the data
if Smothly:
    tt_util = [sum(x)/len(x) for x in chunked(tt_util, Smothly_Interval)]
    gc_util = [sum(x)/len(x) for x in chunked(gc_util, Smothly_Interval)]
    length = len(tt_util)
    time = time[:length]
    for i in range(0, len(time)):
        time[i] *= Smothly_Interval

plt.plot(time, tt_util, color='blue', label='total_util')
plt.plot(time, gc_util, color='red', label='gc_util')
plt.legend()
plt.show()
