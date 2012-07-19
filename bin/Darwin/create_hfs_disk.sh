#!/bin/sh

SZ_G=${1}

hdiutil create -type SPARSE -fs 'Case-sensitive Journaled HFS+' -size ${SZ_G}g  ./disk${SZ_G}g

#mv ./disk${SZ_G}g.sparseimage ./disk${SZ_G}g





