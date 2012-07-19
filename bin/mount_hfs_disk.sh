#!/bin/bash

idx=1

for i in `find /disk -name "*.sparseimage"`
do
    echo "mount ${i} to /Volumes/disk${idx}"
    hdiutil attach ${i} -mountpoint /Volumes/disk${idx}
    idx=$((${idx} + 1))
done


