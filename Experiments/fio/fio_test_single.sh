#!/bin/bash

start_time=$(date +%s)

fio frontground.fio &
pid1=$!
echo $pid1

wait $pid1

end_time=$(date +%s)
cost=$((end_time-start_time))
echo "Total cost in $cost seconds."