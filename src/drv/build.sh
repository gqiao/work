#!/bin/sh

H=${PWD}

HOST=${H}/`uname -m`
mkdir -p ${HOST}
cd ${H}/diag
CROSS_COMPILE= ARCH=x86 make
cp -fr ./diag.ko ${HOST}/
make clean


ARM=${H}/ARM
mkdir -p ${ARM}
cd ${H}/diag
make KDIR=/work/src/${BRANCH_NAME}.${GERRIT_SERVER}/kernel/android-2.6.35
cp -fr ./diag.ko ${ARM}/
make clean

