#!/bin/sh

usage() {
    echo "$0 -s|signed -b|--bootimg -z|--zimg -h|--help"
    echo "Example:"
    echo "$0 --signed --bootimg <boot.img> --zimg <zImage>"
}

if [ -z ${4} ]; then
    usage
    exit 1
fi

SIGNED="no"

args=`getopt -o hsbz: -l help,signed,bootimg,zimg: -- "$@"`

while true; do
    case "${1}" in
	-s|--signed)
	    SIGNED="yes"
	    shift ;;
	-b|--bootimg)
	    shift
	    BIMG="${1}"
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


cp ${BIMG} tmp_boot.img
split_bootimg.pl tmp_boot.img

if [ ${SIGNED} = "yes" ]; then
    echo "delete signed info on ramdisk"
    dd if=./tmp_boot.img-ramdisk.gz of=./ramdisk.new.gz bs=1 skip=288
    cp -f ./ramdisk.new.gz ./tmp_boot.img-ramdisk.gz
fi


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
    echo "repack boot.img Success!"
else
    echo "repack boot.img Failed!"
fi


