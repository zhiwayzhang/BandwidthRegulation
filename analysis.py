import pandas as pd
from matplotlib import pyplot as plt
from more_itertools import chunked

data = pd.read_csv('data.csv')

time = data['time']
tt_util = data['total_util']
gc_util = data['gc_util']

# smothly handle the data
tt_util = [sum(x)/len(x) for x in chunked(tt_util, 5)]
gc_util = [sum(x)/len(x) for x in chunked(gc_util, 5)]

length = len(tt_util)
time = time[:length]
for i in range(0, len(time)):
    time[i]*=5

plt.plot(time, tt_util, color='blue')
plt.plot(time, gc_util, color='red')

plt.show()
