#!/bin/bash

StartFio() {
	fio $1 2>&1 > ./$1.log
	local end_time=$(date +%s)
	echo "End Time of $1 is $end_time"
	local cost=$((end_time-start_time))
	echo "Total cost of $1 is $cost seconds."
}

start_time=$(date +%s)
echo "Start At $start_time"
StartFio frontground.fio &
pid1=$!
echo $pid1

StartFio background.fio &
pid2=$!
echo $pid2

wait $pid1 $pid2
echo "Test Complete"