#!/bin/sh

#-pthread -I/usr/local/include/guile/2.0  -L/usr/local/lib -lguile-2.0 -lgc
gcc `pkg-config --cflags --libs guile-2.0` $@ *.c 

