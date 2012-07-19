#!/bin/bash

prog=$0

names=(   "boot"    "rootfs"  "data")
types=(   "0x0C,*"  "L"         "L" )
formats=( "vfat"    "ext2"    "ext3")
sizes=(      38       95         "" )

function make_fs {
	p_fmt=$1
	p_name=$2
	p_part=$3

	if [ ${p_name} = "" ]; then
		echo "${prog}: No partition name specified for partition ${p_part}"
		exit 1;
	fi

	if [ ${p_fmt} = "vfat" ]; then
		simpcmd "Format ${p_name}" sudo mkfs.vfat -F 32 -n ${p_name} ${DRIVE}${PARTSEP}$p_part
	elif [ ${p_fmt} = "ext3" ]; then
		simpcmd "Format ${p_name}" sudo mkfs.ext3 -m 1 -j -L ${p_name} ${DRIVE}${PARTSEP}$p_part
	elif [ ${p_fmt} = "ext2" ]; then
		simpcmd "Format ${p_name}" sudo mkfs.ext3 -m 1 -L ${p_name} ${DRIVE}${PARTSEP}$p_part
	else
		echo "${prog}: Unknown partition type \"${p_fmt}\""
		exit 1;
	fi
}


function banner {
    echo ""
    echo "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv"
    echo ">> $*"
    echo "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
    echo ""
}


function fail {
    echo "$1 failed"
    exit $2
}  
          

function simpcmd {
    name=$1
    shift
    $* || fail "$name" $?
}
          

function unmountAll {
    sudo umount ${DRIVE}${PARTSEP}1 >& /dev/null
    sudo umount ${DRIVE}${PARTSEP}2 >& /dev/null
    sudo umount ${DRIVE}${PARTSEP}3 >& /dev/null
    sudo umount ${DRIVE}${PARTSEP}4 >& /dev/null
    sudo umount ${DRIVE}${PARTSEP}5 >& /dev/null
    sudo umount ${DRIVE}${PARTSEP}6 >& /dev/null
    sudo umount ${DRIVE}${PARTSEP}7 >& /dev/null
    sudo umount ${DRIVE}${PARTSEP}8 >& /dev/null
    sudo umount ${DRIVE}${PARTSEP}9 >& /dev/null
}
          

if [ $# -lt 1 ]; then
    echo "Usage: $prog drive"
    echo "drive is the /dev file for the SD card; like /dev/sdc, NOT sdc1."
    echo "Make sure you get the right one, as all contents will be destroyed."
    exit 1
fi


DRIVE=$1

if [ ! -b ${DRIVE} ]; then
    echo "$prog: $DRIVE is not a block device."
    echo "Check that you have specified the device correctly."
    exit 1
fi


SIZE_B=`sudo fdisk -l $DRIVE | grep '^Disk.*bytes$' | awk '{print $5}'`
if [ "$SIZE_B" = "" ]; then
    echo "$prog: failed to read the size of $DRIVE."
    echo "Check that you have specified the device correctly."
    exit 1
fi
SIZE_MB=`expr $SIZE_B / 1000000`

if [ "$SIZE_MB" -lt 1950 ]; then
    echo "Device $DRIVE is only $SIZE_MB MB -- this is too small."
    echo "Make sure you have a 2GB MicroSD card, and that you have"
    echo "specified the device correctly."
    exit 1
elif [ "$SIZE_MB" -gt 9000 ]; then
    echo "Device $DRIVE is $SIZE_MB MB -- it looks like you are trying to"
    echo "wipe your hard drive!  Check that you have specified the device"
    echo "correctly."
    exit 1
elif [ "$SIZE_MB" -gt 1980 ]; then
    echo "Device $DRIVE is $SIZE_MB MB -- we normally use a 2GB card."
    echo -n "Do you want to use this device anyway? [y/n]: "
    read goOn
    if [ $goOn != "y" ]; then
        echo "Aborted"
        exit 1
    fi
fi


echo "${DRIVE} is $SIZE_MB MB."
echo "Are you sure you want to completely wipe all the contents of ${DRIVE}?"
echo -n "Enter \"yes\" to continue: "
read doWipe
if [ $doWipe != "yes" ]; then
    echo "Aborted"
    exit 1
fi


banner "Format ${DRIVE}."


# Partitions on MMC devices are like /dev/mmcblk0p2
PARTSEP=
if echo "$DRIVE" | grep -q mmc ; then
    PARTSEP='p'
fi


unmountAll


# Wipe the drive completely.  Well, I warned you.
simpcmd "Wipe ${DRIVE}" sudo dd if=/dev/zero of=$DRIVE bs=1024 count=1024

CYLINDERS=`echo ${SIZE_B}/128/32/512 | bc`

{
    for i in {0..2}; do
        echo ,${sizes[i]},${types[i]}
    done

    echo ,,E

    for (( i=3; i<${#names[*]}; i++ )); do
        echo ,${sizes[i]},${types[i]}
    done
} | simpcmd "Partition" sudo sfdisk -D -H 128 -S 32 -C $CYLINDERS $DRIVE
sleep 1s


unmountAll


for (( i=0; i<${#names[*]}; i++ )); do
    p=$(( i + 1 ))
    if [ $p -gt 3 ]; then
        p=$(( p + 1 ))
    fi

    make_fs ${formats[i]} "${names[i]}" ${p}
done

sleep 1s


unmountAll


banner "Partition Table"

for (( i=0; i<${#names[*]}; i++ )); do
    p=$(( i + 1 ))
    if [ $p -gt 3 ]; then
        p=$(( p + 1 ))
    fi

    sz=${sizes[i]}
    if [ -z "$sz" ]; then
        sz="rem"
    fi
    actb=`sudo blockdev --getsize64 ${DRIVE}${PARTSEP}${p}`
    actmb=$(( actb / 1000000 ))
    actmib=$(( actb / 1024 / 1024 ))
    printf "%-12s%-16s %-8s %3s cyl %4d MB %4d MiB\n"                 \
        ${names[i]} ${DRIVE}${PARTSEP}${p} ${formats[i]} $sz $actmb $actmib
done

banner "Format completed."

exit 0

