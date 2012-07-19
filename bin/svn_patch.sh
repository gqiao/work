#!/bin/sh

T=`date +%Y%m%d-%H`

svn diff > ${T}.patch

