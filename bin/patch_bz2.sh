#!/bin/sh

# ../patch-...bz2
PATCH_BZ2=${1}

bzcat ${PATCH_BZ2} | patch -p1

