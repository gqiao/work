#!/bin/sh

echo "you can free passwd by passwd_free.sh ${1}"
ssh -X -L 5900:localhost:5900 ${1}

