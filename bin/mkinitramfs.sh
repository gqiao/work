#!/bin/sh

#UNAME=2.6.35.10+
UNAME=${1}

sudo mkinitramfs -o /boot/initrd.img-${UNAME}   /lib/modules/${UNAME}

