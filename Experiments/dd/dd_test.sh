#!/bin/bash

StartDD() {
    dd if=/dev/zero of=/tmp/test/$1 bs=$2 count=$3 oflag=direct 2>&1 > ./$1.log
	local end_time=$(date +%s)
	echo "End Time of $1 is $end_time"
	local cost=$((end_time-start_time))
	echo "Total cost of $1 is $cost seconds."
}

start_time=$(date +%s)
echo "Start At $start_time"
StartDD dd.file1 1M 1800 &
pid1=$!
echo $pid1

StartDD dd.file2 1M 900 &
pid2=$!
echo $pid2

wait $pid1 $pid2
echo "Test Complete"