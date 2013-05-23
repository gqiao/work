#!/bin/bash

CMD=`basename ${1}`
nohup ${1} > ${CMD}.log 2>&1 &

echo "[ PID = ${!} ] ${CMD}.log"

exit


