#!/bin/bash

prog=$0

names=(   "boot"    "rootfs" )
types=(   "0x0C,*"  "L"      )
formats=( "vfat"    "ext2"   )
sizes=(      38           "" )

function make_fs {
    F=$1
    N=$2
    P=$3

    if [ ${F} = "vfat" ]; then
	sudo mkfs.vfat -F 32 -n    ${N} ${DRIVE}${P}
    elif [ ${F} = "ext3" ]; then
	sudo mkfs.ext3 -m 1 -j -L  ${N} ${DRIVE}${P}
    elif [ ${F} = "ext2" ]; then
	sudo mkfs.ext3 -m 1 -L     ${N} ${DRIVE}${P}
    fi
}


if [ $# -lt 1 ]; then
    echo "Usage: $prog drive"
    echo "drive is the /dev file for the SD card; like /dev/sdc, NOT sdc1."
    echo "Make sure you get the right one, as all contents will be destroyed."
    exit 1
fi


DRIVE=$1


sudo umount /media/*
sudo dd if=/dev/zero of=${DRIVE} bs=1024 count=1024

SIZE_BYTES=`sudo fdisk -l $DRIVE | grep '^Disk.*bytes$' | awk '{print $5}'`
CYLINDERS=`echo ${SIZE_BYTES}/128/32/512 | bc`

{
    for i in {0..2}; do
        echo ,${sizes[i]},${types[i]}
    done

    echo ,,E

    for (( i=3; i<${#names[*]}; i++ )); do
        echo ,${sizes[i]},${types[i]}
    done
} | sudo sfdisk -D -H 128 -S 32 -C $CYLINDERS $DRIVE

sleep 1s

for (( i=0; i<${#names[*]}; i++ )); do
    p=$(( i + 1 ))
    if [ $p -gt 3 ]; then
        p=$(( p + 1 ))
    fi

    make_fs ${formats[i]} "${names[i]}" ${p}
done







