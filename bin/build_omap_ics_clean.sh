#!/bin/bash

cd /work/src/${TARGET_NAME}.${BRANCH_NAME}.${SERVER_NAME}

rm -fr bootable/bootloader/x-loader/include/config.h
rm -fr bootable/bootloader/u-boot/include/config.h
rm -fr kernel/android-3.0/.config

