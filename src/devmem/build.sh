#!/bin/sh

mkdir -p ARM
${CROSS_COMPILE}gcc -Wall -Os -static devmem.c -o ./ARM/devmem
gcc devmem.c -o devmem

