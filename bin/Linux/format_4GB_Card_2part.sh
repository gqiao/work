#! /bin/sh
# mkcard.sh v0.3
# (c) Copyright 2009 Graeme Gregory <dp@xora.org.uk>
# Licensed under terms of GPLv2

DRIVE=$1


function unmountAll {
    sudo umount ${DRIVE}1 >& /dev/null
    sudo umount ${DRIVE}2 >& /dev/null
    sudo umount ${DRIVE}3 >& /dev/null
    sudo umount ${DRIVE}4 >& /dev/null
    sudo umount ${DRIVE}5 >& /dev/null
    sudo umount ${DRIVE}6 >& /dev/null
    sudo umount ${DRIVE}7 >& /dev/null
    sudo umount ${DRIVE}8 >& /dev/null
    sudo umount ${DRIVE}9 >& /dev/null
}

unmountAll

dd if=/dev/zero of=$DRIVE bs=1024 count=1024

SIZE=`fdisk -l $DRIVE | grep Disk | awk '{print $5}'`
echo DISK SIZE - $SIZE bytes

CYLINDERS=`echo $SIZE/255/63/512 | bc`
echo CYLINDERS - $CYLINDERS

{
echo ,307,0x0C,*
echo ,,,-
} | sfdisk -D -H 255 -S 63 -C $CYLINDERS $DRIVE

if [ -b ${DRIVE}1 ]; then
	mkfs.vfat -F 32 -n "boot" ${DRIVE}1
else
	if [ -b ${DRIVE}p1 ]; then
		mkfs.vfat -F 32 -n "boot" ${DRIVE}p1
	else
		echo "Cant find boot partition in /dev"
	fi
fi

unmountAll
sleep 1s


if [ -b ${DRIVE}2 ]; then
	mke2fs -j -L "rootfs" ${DRIVE}2
else
	if [ -b ${DRIVE}p2 ]; then
		mke2fs -j -L "rootfs" ${DRIVE}p2
	else
		echo "Cant find rootfs partition in /dev"
	fi
fi

sync

