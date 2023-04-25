#!/bin/bash

start_time=$(date +%s)

dd if=/dev/zero of=/tmp/test/file1 bs=1M count=1800 oflag=direct &
pid1=$!
echo $pid1

dd if=/dev/zero of=/tmp/test/file2 bs=1M count=1800 oflag=direct &
pid2=$!
echo $pid2

wait $pid1 $pid2

end_time=$(date +%s)
cost=$((end_time-start_time))
echo "Total cost in $cost seconds."