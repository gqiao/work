#!/bin/sh

if [ "`uname -m`" = "x86_64" ];then
    B=amd64
else
    B=i386
fi

wget http://cdimage.debian.org/cdimage/release/current-live/${B}/usb-hdd/debian-live-6.0.3-${B}-gnome-desktop.img


dd if=${1} of=${2} bs=8M


