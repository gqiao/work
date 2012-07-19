#!/bin/sh

adb push ./devmem      /data/
adb push ./busybox     /data/
adb push ./memtest.sh  /data/
adb shell "START=0x9b000000  END=0x9b00000F /data/busybox sh /data/memtest.sh"

