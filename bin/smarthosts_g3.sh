#!/bin/sh

TDIR=`mktemp -d`

cd ${TDIR}
wget https://smarthosts.googlecode.com/svn/trunk/mobile_devices/hosts
adb shell "mount -o remount /dev/block/mtdblock3 /system"
adb push hosts /system/etc/hosts
