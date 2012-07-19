#!/bin/sh
usage() {
    echo "$0 update.zip"
    echo "Example:"
    echo "$0 ./erdf.s1.zip "
    exit 1
}

if [ ! -f ${1} ]; then
    usage
    exit 1
fi

ACCLAIM_UPDATE=${1}
# if [ -d /misc ]; then
# mount /dev/blocks/mmcblk0p7 /mnt/media
if adb shell "mount" | grep "misc"; then
    adb shell "rm -fr /misc/iectest"
    adb push ${ACCLAIM_UPDATE} /misc/factory.zip
else
    adb shell "mount -t ext4 /dev/block/mmcblk0p7 /mnt/media"
    #adb shell "rm -fr /mnt/media/iectest"
    adb push ${ACCLAIM_UPDATE} /mnt/media/factory.zip
fi

adb shell "echo -n "\$\$\$\$" > /bootdata/BootCnt"
adb shell "sync"
adb reboot








