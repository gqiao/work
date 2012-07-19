#!/bin/sh

if [ -z ${1} ]; then
    echo "using:"
    adb devices
else
    export ANDROID_SERIAL=${1}
fi

S_DAT=src_data.dat
D_DAT=dst_data.dat

trap 'rm -fr ${S_DAT} ${D_DAT}' exit 0

dd if=/dev/zero of=${S_DAT} bs=1G count=6
i=0
while true
do
    echo "[" $i "]"
    
    T=`date +%Y%m%d-%H.%M.%S`
    echo "${T} test usb adb copy: `du -sh ${S_DAT} | awk '{print $1}'`"
    adb shell "rm -fr /sdcard/test.dat"
    echo -n "push to dev:   "
    adb push ${S_DAT} /sdcard/test.dat
    echo -n "pull from dev: "
    adb pull /sdcard/test.dat ${D_DAT}
    cmp ${S_DAT} ${D_DAT}

    rc=$?
    T=`date +%Y%m%d-%H.%M.%S`
    if [ "$rc" -eq 0 ] ; then
	echo " => result: ${T} PASS"
    else
	echo " => result: ${T} FAIL"
    fi
    
    i=$((${i} + 1))
done

