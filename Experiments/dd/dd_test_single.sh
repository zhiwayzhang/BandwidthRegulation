#!/bin/bash

start_time=$(date +%s)

dd if=/dev/zero of=/tmp/test/file1 bs=180M count=10 oflag=direct &
pid1=$!
echo $pid1

wait $pid1

end_time=$(date +%s)
cost=$((end_time-start_time))
echo "Total cost in $cost seconds."