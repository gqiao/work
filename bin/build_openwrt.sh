#!/bin/sh


make package/symlinks
make prereq
make download world
make world -j3
make V=99
echo "make kernel_menuconfig"

