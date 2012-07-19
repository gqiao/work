#!/bin/sh

T=`date +%Y%m%d-%H`

git commit -a -m "${T} @ ${PWD} : another backup: ${1}"

