#!/bin/bash

# ensure it is correctly used
if [ $# -ne 1 ]
then
        cmd=`basename $0`
        echo "Usage: $cmd uRamdisk"
        exit 1
fi

# get the work dir
work_dir=`pwd`

# get the uRamdisk file name
uRamdisk=$1;
if [ ! -f $uRamdisk ]
then
        echo "$uRamdisk is not existed"
        exit 2
fi

# remove the 64 bytes u-boot header, get the ramdisk.gz
dd if=$uRamdisk bs=64 skip=1 of=$work_dir/ramdisk.gz
ramdisk_gz=$work_dir/ramdisk.gz

# uncompress ramdisk.gz to get ramdisk
gunzip $ramdisk_gz
ramdisk=$work_dir/ramdisk


# upack cpio package
ramdisk_dir=$work_dir/ramdisk_dir
mkdir $ramdisk_dir
mv $ramdisk $ramdisk_dir 
tmp_ramdisk=$ramdisk_dir/ramdisk
cd $ramdisk_dir
cpio -idmv < $tmp_ramdisk
rm -f $tmp_ramdisk

