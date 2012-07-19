#!/bin/sh

nc 10.80.104.239 23 <<EOF
root
ifconfig
q
EOF
