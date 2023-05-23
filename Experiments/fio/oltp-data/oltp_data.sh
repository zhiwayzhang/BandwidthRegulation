#!/bin/bash
for i in {0..7}
do
    sudo dd if=/dev/zero of=/tmp/test/oltp$i bs=1M count=768
done

sudo dd if=/dev/zero of=/tmp/test/data$i bs=1M count=6144