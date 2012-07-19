#!/bin/sh

rm -fr ramdisk.new*
dd if=${1} of=./ramdisk.new.gz bs=1 skip=288
cp ramdisk.new.gz ramdisk.bak.gz 
gunzip -d ./ramdisk.new.gz
rm -fr ramdisk_root
mkdir ramdisk_root
cd    ramdisk_root
cpio -idmv < ../ramdisk.new

