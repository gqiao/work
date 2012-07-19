#!/system/bin/sh
/sbin/busybox --install /sbin

/sbin/busybox --install /system/bin
ln -s /sbin/busybox /system/bin/sh
ln -s /sbin/busybox /system/bin/busybox
mkdir -p /lib/modules/$(uname -r)

# mkbootfs clears the executable bit on all files not in bin directories
chmod -R +x /mfg
sed -n -e 's/^ro.build.description=//p' /default.prop > /tmp/mfg_version

if [ -x /misc/test/run_factory_cmd ] ; then
    /misc/test/run_factory_cmd 
fi
