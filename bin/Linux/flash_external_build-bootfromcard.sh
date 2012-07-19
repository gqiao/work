#!/bin/bash


BOOT_SOURCE="mydroid"

 
echo "Flashing boot"
#cp $BOOT_SOURCE/bootable/bootloader/x-loader/MLO /media/boot/.
cp $BOOT_SOURCE/MLO /media/boot/

#cp $BOOT_SOURCE/bootable/bootloader/u-boot/u-boot.bin /media/boot/.
cp $BOOT_SOURCE/u-boot.bin /media/boot/.

#cp $BOOT_SOURCE/kernel/android-2.6.29/arch/arm/boot/uImage /media/boot/.
cp $BOOT_SOURCE/uImage /media/boot/.

echo "Flashing rootfs"
cp -Rfp myfs/* /media/rootfs/.

pwd | rev | cut -d "/" -f2 | rev > /media/rootfs/system/build_number

chmod --recursive 777 /media/rootfs/*

umount /media/boot
umount /media/rootfs

