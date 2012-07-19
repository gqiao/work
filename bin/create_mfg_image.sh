#!/bin/sh
HERE=`pwd`
ROOT="$HERE/../../.."
rm -r -f mfg
mkdir mfg
# Build tools from source
mfg_tools/build.sh

# copy root files to the mfg image
cp -a "$ROOT"/out/target/product/blaze/recovery/root/* mfg/

# copy busybox to /sbin
install ../misc/busybox-1.18.4/busybox mfg/sbin

# copy IEC tests to /sbin
# iec_bin/* could probably be permanently moved into testutils/
cp -a iec_bin/* mfg/sbin/

# copy the init.rc script to /
cp init.rc mfg/

# create an fstab for use by Busybox
awk '/^[^#]/{print $3 "\t" $1 "\t" $2 "\tdefaults,noatime"}' mfg/etc/recovery.fstab >mfg/etc/fstab

# use busybox as a shell
ln -s busybox mfg/sbin/sh
mkdir mfg/system/bin
mkdir mfg/system/lib

# copy linker for binary requiring so libs
cp -fp "$ROOT"/out/target/product/blaze/system/bin/linker mfg/system/bin
#cp -fp "$ROOT"/out/target/product/blaze/system/lib/libdvm.so mfg/system/lib

# copy the required so libs for the testutils
#cp -a "$ROOT"/out/target/product/blaze/system/lib mfg/system/
cp -a "$ROOT"/out/target/product/blaze/system/etc/dhcpcd mfg/system/etc
cp -fp "$ROOT"/out/target/product/blaze/system/bin/dhcpcd mfg/system/bin
cp -fp "$ROOT"/out/target/product/blaze/system/bin/minizip mfg/system/bin

# copy ALSA util and configuration files
cp -fp "$ROOT"/out/target/product/blaze/system/bin/alsa_aplay mfg/system/bin
cp -fp "$ROOT"/out/target/product/blaze/system/bin/alsa_amixer mfg/system/bin
mkdir -p mfg/system/usr/share
cp -a "$ROOT"/out/target/product/blaze/system/usr/share/alsa/ mfg/system/usr/share
cp -a "$ROOT"/out/target/product/blaze/system/etc/asound.conf mfg/etc

# create the /system/etc folder and copy the wifi driver
cp -a "$ROOT"/out/target/product/blaze/system/etc/wifi mfg/system/etc
# replace the tiwlan.ini file with the modified one provided by TI
cp -a tiwlan.ini mfg/system/etc/wifi

# copy the TI wifi tools to /sbin
cp -fp "$ROOT"/out/target/product/blaze/system/bin/wlan* mfg/sbin
cp -p mfg/sbin/sh mfg/system/bin/

NEEDED_SHARED_LIBS='
libasound.so
libbinder.so
libc.so
libcutils.so
libdl.so
libhardware_legacy.so
liblog.so
libm.so
libnetutils.so
libstdc++.so
libutils.so
libwpa_client.so
libz.so
'
for lib in $NEEDED_SHARED_LIBS; do
	cp -a "$ROOT"/out/target/product/blaze/system/lib/"$lib" mfg/system/lib
done

#copy the testutils to /mfg
cp -a testutils mfg/mfg

# create the /data/misc/wifi folder
mkdir -p mfg/data/misc/wifi

# create the ramdisk and the boot image
"$ROOT"/out/host/linux-x86/bin/mkbootfs mfg/ | "$ROOT"/out/host/linux-x86/bin/minigzip > mfg-ramdisk.img
"$ROOT"/vendor/bn/build/scripts/SignMRD
"$ROOT"/out/host/linux-x86/bin/mkbootimg  --kernel "$ROOT"/kernel/android-2.6.35/arch/arm/boot/zImage  --ramdisk mfg-ramdisk_signed.img --cmdline "console=ttyO0,115200n8 mem=456M@0x80000000 mem=512M@0xA0000000 init=/init vram=32M omapfb.vram=0:16M" --base 0x81000000 --pagesize 4096 --output mfg_boot.img
rm mfg-ramdisk_signed.img
TEMPDIR=`mktemp -d`
trap 'rm -r "$TEMPDIR"' 0
mkdir "$TEMPDIR"/iectest
tar --bzip2 -xf prebuilt/iectest.tar.bz2 -C "$TEMPDIR"/iectest
"$ROOT"/out/host/linux-x86/bin/make_ext4fs -s -l 387973120 -b 4096 -L factory factory.img "$TEMPDIR"
# TEMPDIR is cleaned up by trap on exit
