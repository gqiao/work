#!/bin/bash
. /work/bin/env_qt

QUTA2_PATH=$HOME/QUTA2_PKG/
export LD_LIBRARY_PATH=${QUTA2_PATH}/libs:$LD_LIBRARY_PATH
export PATH=$PATH:$HOME/QUTA2_PKG/tools/
cd $QUTA2_PATH
#rm -r $QUTA2_PATH/offlogs/*
#gksudo chmod a+rw /dev/gpib0
#gksudo gpib_config --minor 0
gksudo ntpdate 172.116.5.13
gksudo hwclock -w
gnome-terminal --geometry=80x24-0-25 -e ./QUTA
