#!/bin/sh

usage() {
    echo "$0 -r|--ramdisk -z|--zimg -h|--help"
    echo "Example:"
    echo "$0 --ramdisk <ramdisk.gz> --zimg <zImage>"
}

if [ -z ${4} ]; then
    usage
    exit 1
fi

args=`getopt -o hrz: -l help,ramdisk,zimg: -- "$@"`

while true; do
    case "${1}" in
	-r|--ramdisk)
	    shift
	    RIMG="${1}"
	    shift ;;
	-z|--zimg)
	    shift
	    ZIMG="${1}"
	    shift ;;
	-h|--help)
	    usage
	    exit 0 ;;
	*) break ;;
    esac
done


cp ${RIMG} tmp_boot.img-ramdisk.gz

KERNEL_CMDLINE="androidboot.console=ttyO0 console=ttyO0,115200n8 init=/init rootwait vram=16M omapfb.vram=0:16M"
KERNEL_BASE="0x80080000"
KERNEL_PAGESIZE="4096"

mkbootimg                              \
    --kernel   "${ZIMG}"               \
    --ramdisk  tmp_boot.img-ramdisk.gz \
    --cmdline  "$KERNEL_CMDLINE"       \
    --base     "$KERNEL_BASE"          \
    --pagesize "$KERNEL_PAGESIZE"      \
    --output boot.img

if [ $? -eq 0 ]; then
    rm -fr tmp_boot.img*
    echo "repack ${RIMG} Success!"
else
    echo "repack ${RIMG} Failed!"
fi


